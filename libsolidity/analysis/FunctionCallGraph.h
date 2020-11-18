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

#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <vector>

namespace solidity::frontend
{

class FunctionCallGraphBuilder: private ASTConstVisitor
{
public:
	struct Node;

	struct NodeCompare
	{
		// this member is required to let container be aware that
		// comparator is capable of dealing with types other than key
		using is_transparent = std::true_type;

		bool operator()(std::shared_ptr<Node> const _lhs, std::shared_ptr<Node> const _rhs) const
		{
			return _lhs->callable < _rhs->callable;
		}
		bool operator()(std::shared_ptr<Node> const _lhs, CallableDeclaration const* _rhs) const
		{
			return _lhs->callable < _rhs;
		}
		bool operator()(CallableDeclaration const* _lhs, std::shared_ptr<Node> const _rhs) const
		{
			return _lhs < _rhs->callable;
		}
	};

	typedef std::set<std::shared_ptr<Node>, NodeCompare> NodeSet;

	struct Node
	{
		Node(CallableDeclaration const* _callable): callable(_callable) {}
		// Definition of this function/modifier/event, might be nullptr for
		// implicit constructors
		CallableDeclaration const* callable = nullptr;

		// Calls that this function/modifier/event makes to other callables
		NodeSet calls;
	};

	struct ContractCallGraph
	{
		ContractCallGraph(ContractDefinition const& _contract) :contract(_contract) {}

		/// Contract for which this is the graph
		ContractDefinition const& contract;
		/// Calls made at creation time (constructor, state variables, ..)
		NodeSet creationCalls;
		/// Calls possible at runtime (public functions, events)
		NodeSet runtimeCalls;
	};

	std::shared_ptr<ContractCallGraph> const create(ContractDefinition const& _contract);

private:
	bool visit(Identifier const& _identifier) override;
	bool visit(NewExpression const& _newExpression) override;
	bool visit(MemberAccess const& _memberAccess) override;

	void visitCallable(CallableDeclaration const* _callable);
	void visitConstructor(ContractDefinition const& _contract);

	ContractDefinition const* m_contract = nullptr;
	std::shared_ptr<Node> m_currentNode = nullptr;
	NodeSet* m_nodes = nullptr;
};

}
