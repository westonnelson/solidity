/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/analysis/FunctionCallGraph.h>

using namespace std;

namespace solidity::frontend
{


shared_ptr<FunctionCallGraphBuilder::ContractCallGraph> const FunctionCallGraphBuilder::create(ContractDefinition const& _contract)
{
	m_contract = &_contract;

	std::shared_ptr<ContractCallGraph> graph= make_shared<ContractCallGraph>(_contract);

	// Create graph for constructor, state vars, etc
	m_nodes = &graph->creationCalls;
	visitConstructor(_contract);


	// Create graph for all publicly reachable functions
	m_nodes = &graph->runtimeCalls;
	for (auto& [hash, functionType]: _contract.interfaceFunctionList())
		if (auto const* funcDef = dynamic_cast<FunctionDefinition const*>(&functionType->declaration()))
			if (!m_nodes->count(funcDef))
				visitCallable(funcDef);

	m_contract = nullptr;
	m_nodes = nullptr;
	solAssert(m_currentNode == nullptr, "Current node not properly reset.");

	return graph;
}

bool FunctionCallGraphBuilder::visit(Identifier const& _identifier)
{
	if (auto const* callable = dynamic_cast<CallableDeclaration const*>(_identifier.annotation().referencedDeclaration))
	{
		if (*_identifier.annotation().requiredLookup == VirtualLookup::Virtual)
			callable = &callable->resolveVirtual(*m_contract);

		solAssert(
			*_identifier.annotation().requiredLookup != VirtualLookup::Super,
			"Only MemberAccess can have lookup 'super'"
		);

		// Ignore calls to other contract
		if (callable->annotation().contract && !m_contract->derivesFrom(*callable->annotation().contract) && !callable->annotation().contract->isLibrary())
			return true;

		if (!m_nodes->count(callable))
			visitCallable(callable);

		m_currentNode->calls.emplace(*m_nodes->find(callable));
	}

	return true;
}

bool FunctionCallGraphBuilder::visit(NewExpression const& _newExpression)
{
	if (ContractType const* contractType = dynamic_cast<ContractType const*>(_newExpression.typeName().annotation().type))
	{
		CallableDeclaration const* callable = contractType->contractDefinition().constructor();

		if (!m_nodes->count(callable))
			m_nodes->emplace(make_shared<Node>(callable));
	}

	return true;
}

bool FunctionCallGraphBuilder::visit(MemberAccess const& _memberAccess)
{
	if (auto const* callable = dynamic_cast<CallableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
	{
		if (*_memberAccess.annotation().requiredLookup == VirtualLookup::Super)
		{
			ContractDefinition const* super =
				m_currentNode->callable ?
				m_currentNode->callable->annotation().contract :
				m_contract;

			callable = &callable->resolveVirtual(*m_contract, super);
		}

		solAssert(
			*_memberAccess.annotation().requiredLookup != VirtualLookup::Virtual,
			"MemberAccess cannot have lookup 'virtual'"
		);

		// Ignore calls to other contracts
		if (callable->annotation().contract && !m_contract->derivesFrom(*callable->annotation().contract) && !callable->annotation().contract->isLibrary())
			return true;

		if (!m_nodes->count(callable))
			visitCallable(callable);
	}

	return true;
}

void FunctionCallGraphBuilder::visitCallable(CallableDeclaration const* _callable)
{
	solAssert(!m_nodes->count(_callable), "");

	auto previousCallable = m_currentNode;
	m_nodes->emplace(m_currentNode = make_shared<Node>(_callable));
	_callable->accept(*this);
	m_currentNode = previousCallable;
}

void FunctionCallGraphBuilder::visitConstructor(ContractDefinition const& _contract)
{
	auto previousNode = m_currentNode;
	m_nodes->emplace(m_currentNode = make_shared<Node>(_contract.constructor()));
	if (previousNode)
		previousNode->calls.emplace(m_currentNode);

	if (_contract.annotation().linearizedBaseContracts.size() > 1)
		visitConstructor(*_contract.annotation().linearizedBaseContracts[1]);

	for (auto const* stateVar: _contract.stateVariables())
		stateVar->accept(*this);

	if (_contract.constructor())
		_contract.constructor()->accept(*this);

	m_currentNode = previousNode;
}

}
