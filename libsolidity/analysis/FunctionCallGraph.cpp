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
using namespace solidity::frontend;

namespace {

bool nodeSet(FunctionCallGraphBuilder::Node const& _node)
{
	if (auto node = std::get_if<FunctionCallGraphBuilder::SpecialNode>(&_node))
		return *node != FunctionCallGraphBuilder::SpecialNode::Unset;

	return true;
}

}

namespace solidity::frontend
{


shared_ptr<FunctionCallGraphBuilder::ContractCallGraph> const FunctionCallGraphBuilder::create(ContractDefinition const& _contract)
{
	m_contract = &_contract;

	m_graph = make_shared<ContractCallGraph>(_contract);

	// Create graph for constructor, state vars, etc
	m_currentNode = SpecialNode::CreationRoot;
	m_currentDispatch = SpecialNode::CreationDispatch;
	visitConstructor(
		_contract,
		_contract.annotation().linearizedBaseContracts.cbegin() + 1,
		_contract.annotation().linearizedBaseContracts.cend()
	);
	m_currentNode = SpecialNode::Unset;
	m_currentDispatch = SpecialNode::RuntimeDispatch;

	// Create graph for all publicly reachable functions
	for (auto& [hash, functionType]: _contract.interfaceFunctionList())
		if (auto const* funcDef = dynamic_cast<FunctionDefinition const*>(&functionType->declaration()))
			if (!m_graph->edges.count(funcDef))
				visitCallable(funcDef);

	m_contract = nullptr;
	solAssert(!nodeSet(m_currentNode), "Current node not properly reset.");

	return m_graph;
}

// Function not called right away -> dispatch
// FunctionCall without ref -> dispatch
//
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

		// Ignore all calls that are not internal
		if (auto funcType = dynamic_cast<FunctionType const*>(callable->type()))
			if (funcType->kind() != FunctionType::Kind::Internal)
				return true;

		if (!m_graph->edges.count(callable))
		{
			// Create edge to creation dispatch (possibly will be removed if
			// function is called in FunctionCall visitor)
			m_graph->edges.emplace(m_currentDispatch);
			visitCallable(callable);
		}

		solAssert(nodeSet(m_currentNode), "");
	}

	return true;
}

bool FunctionCallGraphBuilder::visit(NewExpression const& _newExpression)
{
	if (ContractType const* contractType = dynamic_cast<ContractType const*>(_newExpression.typeName().annotation().type))
		m_graph->createdContracts.emplace(&contractType->contractDefinition());

	return true;
}

bool FunctionCallGraphBuilder::visit(MemberAccess const& _memberAccess)
{
	if (auto const* callable = dynamic_cast<CallableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
	{
		if (*_memberAccess.annotation().requiredLookup == VirtualLookup::Super)
		{
			CallableDeclaration const* callableNode = nullptr;

			if (auto node = std::get_if<ASTNode const*>(&m_currentNode))
				callableNode = dynamic_cast<CallableDeclaration const*>(*node);

			ContractDefinition const* super =
				callableNode ?
				callableNode->annotation().contract :
				m_contract;

			callable = &callable->resolveVirtual(*m_contract, super);
		}

		solAssert(
			*_memberAccess.annotation().requiredLookup != VirtualLookup::Virtual,
			"MemberAccess cannot have lookup 'virtual'"
		);

		// Ignore all calls that are not internal
		if (callable->visibility() == Visibility::External)
			return true;

		if (auto funcType = dynamic_cast<FunctionType const*>(callable->type()))
			if (funcType->kind() != FunctionType::Kind::Internal)
				return true;

		if (!m_graph->edges.count(callable))
			visitCallable(callable);
	}

	return true;
}

bool FunctionCallGraphBuilder::visit(FunctionCall const& _functionCall)
{
	_functionCall.expression().accept(*this);
	FunctionCall::listAccept(_functionCall.arguments(), *this);

	auto& functionType = dynamic_cast<FunctionType const&>(*_functionCall.expression().annotation().type);

	if (functionType.hasDeclaration())
	auto [begin, end] = m_graph->edges.equal_range(m_currentDispatch);

	while (begin != end)
		if (dynamic_cast<FunctionDefinition const*>(begin->second)->annotation().type

	return false;
}

void FunctionCallGraphBuilder::visitCallable(CallableDeclaration const* _callable)
{
	solAssert(!m_graph->edges.count(_callable), "");

	auto previousNode = m_currentNode;
	m_currentNode = _callable;

	if (nodeSet(previousNode))
		m_graph->edges.emplace(previousNode, _callable);

	_callable->accept(*this);

	m_currentNode = previousNode;
}

void FunctionCallGraphBuilder::visitConstructor(
	ContractDefinition const& _contract,
	vector<ContractDefinition const*>::const_iterator _start,
	vector<ContractDefinition const*>::const_iterator _end
)
{
	if (_start != _end)
		visitConstructor(**_start, _start + 1, _end);

	for (auto const* stateVar: _contract.stateVariables())
		stateVar->accept(*this);

	if (_contract.constructor())
	{
		m_graph->edges.emplace(m_currentNode, _contract.constructor());
		_contract.constructor()->accept(*this);
	}
}

}
