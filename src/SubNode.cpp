#include "SubNode.h"
#include "NodeRegistrar.h"

REGISTER_NODE_TYPE(NodeType::Sequence, SequenceNode, "Sequence")
REGISTER_NODE_TYPE(NodeType::Selector, SelectorNode, "Selector")
REGISTER_NODE_TYPE(NodeType::Action, ActionNode, "Action")
REGISTER_NODE_TYPE(NodeType::Condition, ConditionNode, "Condition")