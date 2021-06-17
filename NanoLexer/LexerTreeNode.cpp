#include "LexerTreeNode.h"

#include <cassert>
#include <string>

namespace std
{
    bool less<::RegularExpression::Interval>::operator()(const ::RegularExpression::Interval &lhs, const ::RegularExpression::Interval &rhs) const
    {
        if (lhs.getMin() == rhs.getMin())
            return lhs.getMax() < rhs.getMax();
        else
            return lhs.getMin() < rhs.getMin();
    }

    std::hash<::RegularExpression::CharValueNode*>    hash<::RegularExpression::PositionSet>::hasher;

    std::size_t hash<::RegularExpression::PositionSet>::operator()(const ::RegularExpression::PositionSet& s) const
    {
        size_t hash_val = 1;
        for (auto pos : s)
        {
            hash_val += hasher(pos);
            hash_val *= 17;
        };
        return hash_val;
    }

    bool less<::RegularExpression::CharValueNode*>::operator()(const::RegularExpression::CharValueNode * lhs, const::RegularExpression::CharValueNode * rhs) const
    {
        return lhs->getId() < rhs->getId();
    }
}

namespace RegularExpression
{
    std::string char2stringExpr(unsigned char v)
    {
        switch (v)
        {
        case '\n': return "'\\n'";
        case '\t': return "'\\t'";
        case '\v': return "'\\v'";
        case '\b': return "'\\b'";
        case '\r': return "'\\r'";
        case '\f': return "'\\f'";
        case '\a': return "'\\a'";
        case '\'': return "'\\''";
        case '\\': return "'\\\\'";
        default:
            std::string str;
            if (v >= 32 && v <= 126)
            {
                // printable characters
                str = "'";
                str += v;
                str += "'";
            }
            else
            {
                str = std::to_string(v);
            }
            return str;
        }
    }

    ///////////////////////////////////////////////////////////////
    //	class PositionSet

    PositionSet::PositionSet(const PositionSet &s)
        :std::set<CharValueNode*>(s)
    {
        for (auto set : s)
        {
            insert(set);
        }
    }

    ///////////////////////////////////////////////////////////////
    //	class ConcatNode

    ConcatNode::ConcatNode()
    {
    }

    ConcatNode::ConcatNode(std::vector<std::shared_ptr<BaseLexerTreeNode>>&& children)
        :m_Children(std::move(children))
    {
        assert(m_Children[0]);
    }

    void ConcatNode::addChild(std::shared_ptr<BaseLexerTreeNode>	child)
    {
        m_Children.push_back(child);
    }

    bool ConcatNode::nullable()
    {
        for (const auto &child : m_Children)
        {
            if (!child->nullable())
                return false;
        }
        return true;
    }

    std::shared_ptr<BaseLexerTreeNode> ConcatNode::clone() const
    {
        auto concat = std::make_shared<ConcatNode>();
        for (auto child : m_Children)
        {
            concat->m_Children.push_back(child->clone());
            assert(m_Children.back());
        }
        return concat;
    }

    std::string ConcatNode::toDebugString() const
    {
        std::string debug = "(";
        for (auto child : m_Children)
        {
            if (debug.size() > 1)
                debug += ".";
            debug += child->toDebugString();
        }
        debug += ")";
        return debug;
    }

    int ConcatNode::getAcceptLexId() const
    {
        auto minLex = -1;
        for (auto child : m_Children)
        {
            auto lex = child->getAcceptLexId();
            if (minLex == -1 || lex < minLex)
                minLex = lex;
        }
        return minLex;
    }

    void ConcatNode::computeNextPos()
    {
        std::vector<BaseLexerTreeNode*> arrayPrevious;
        for (const auto &child : m_Children)
        {
            for (auto *previous : arrayPrevious)
            {
                for (auto &last : previous->getLastPos())
                {
                    last->addNext(child->getFirstPos());
                }
            }
            if (!child->nullable())
                arrayPrevious.clear();
            arrayPrevious.push_back(child.get());
            child->computeNextPos();
        }
    }

    void ConcatNode::computeFirstPos()
    {
        auto nullable = true;
        for (const auto &child : m_Children)
        {
            child->computeFirstPos();
            if (nullable)
                addFirst(child->getFirstPos());
            if (!child->nullable())
                nullable = false;
        }
    }

    void ConcatNode::computeLastPos()
    {
        auto nullable = true;
        for (auto iter = m_Children.rbegin()
            ; iter != m_Children.rend()
            ; iter++)
        {
            const auto &child = *iter;
            child->computeLastPos();
            if (nullable)
                addLast(child->getLastPos());
            if (!child->nullable())
                nullable = false;
        }
    }

#ifdef TRACE_EXPREG
    void ConcatNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "node CONCAT" << "    First(" << m_FirstPos.toDebugIdList() << ")    Next(" << m_NextPos.toDebugIdList() << ")    Last(" << m_LastPos.toDebugIdList() << ")" << std::endl;
        for (auto child : m_Children)
        {
            child->trace(str, indent + 1);
        }
    }
#endif

    ///////////////////////////////////////////////////////////////
    //	class OrNode

    OrNode::OrNode()
    {
    }

    OrNode::OrNode(std::vector<std::shared_ptr<BaseLexerTreeNode>>&& children)
        :m_Children(std::move(children))
    {
    }

    void OrNode::addChild(std::shared_ptr<BaseLexerTreeNode>	child)
    {
        m_Children.push_back(child);
    }

    bool OrNode::nullable()
    {
        for (const auto &child : m_Children)
        {
            if (child->nullable())
                return true;
        }
        return false;
    }

    std::shared_ptr<BaseLexerTreeNode> OrNode::clone() const
    {
        auto node = std::make_shared<OrNode>();
        for (auto child : m_Children)
        {
            node->m_Children.push_back(child->clone());
        }
        return node;
    }

    std::string OrNode::toDebugString() const
    {
        std::string debug = "(";
        for (auto child : m_Children)
        {
            if (debug.size() > 1)
                debug += "|";
            debug += child->toDebugString();
        }
        debug += ")";
        return debug;
    }

    int OrNode::getAcceptLexId() const
    {
        auto minLex = -1;
        for (auto child : m_Children)
        {
            auto lex = child->getAcceptLexId();
            if (minLex == -1 || lex < minLex)
                minLex = lex;
        }
        return minLex;
    }

    void OrNode::computeNextPos()
    {
        for (const auto &child : m_Children)
        {
            child->computeNextPos();
        }
    }

    void OrNode::computeFirstPos()
    {
        for (const auto &child : m_Children)
        {
            child->computeFirstPos();
            addFirst(child->getFirstPos());
        }
    }

    void OrNode::computeLastPos()
    {
        for (const auto &child : m_Children)
        {
            child->computeLastPos();
            addLast(child->getLastPos());
        }
    }

#ifdef TRACE_EXPREG
    void OrNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "node OR" << "    First(" << m_FirstPos.toDebugIdList() << ")    Next(" << m_NextPos.toDebugIdList() << ")    Last(" << m_LastPos.toDebugIdList() << ")" << std::endl;
        for (auto child : m_Children)
        {
            child->trace(str, indent + 1);
        }
    }
#endif

    ///////////////////////////////////////////////////////////////
    //	class OptionNode

    OptionNode::OptionNode(std::shared_ptr<BaseLexerTreeNode>	child)
        :m_pChild(child)
    {
    }

    bool OptionNode::nullable()
    {
        return true;
    }

    std::shared_ptr<BaseLexerTreeNode> OptionNode::clone() const
    {
        return std::make_shared<OptionNode>(m_pChild->clone());
    }

    std::string OptionNode::toDebugString() const
    {
        std::string debug = "(";
        debug += m_pChild->toDebugString();
        debug += ")?";
        return debug;
    }

    int OptionNode::getAcceptLexId() const
    {
        // cannot contain end node
        return -1;
    }

    void OptionNode::computeNextPos()
    {
        m_pChild->computeNextPos();
        addNext(m_pChild->getNextPos());
    }

    void OptionNode::computeFirstPos()
    {
        m_pChild->computeFirstPos();
        addFirst(m_pChild->getFirstPos());
    }

    void OptionNode::computeLastPos()
    {
        m_pChild->computeLastPos();
        addLast(m_pChild->getLastPos());
    }

#ifdef TRACE_EXPREG
    void OptionNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "node OPTION" << "    First(" << m_FirstPos.toDebugIdList() << ")    Next(" << m_NextPos.toDebugIdList() << ")    Last(" << m_LastPos.toDebugIdList() << ")" << std::endl;
        m_pChild->trace(str, indent + 1);
    }
#endif

    ///////////////////////////////////////////////////////////////
    //	class PlusNode

    PlusNode::PlusNode(std::shared_ptr<BaseLexerTreeNode>	child)
        :m_pChild(child)
    {
    }

    bool PlusNode::nullable()
    {
        return m_pChild->nullable();
    }

    std::shared_ptr<BaseLexerTreeNode> PlusNode::clone() const
    {
        return std::make_shared<PlusNode>(m_pChild->clone());
    }

    std::string PlusNode::toDebugString() const
    {
        std::string debug = "(";
        debug += m_pChild->toDebugString();
        debug += ")+";
        return debug;
    }

    int PlusNode::getAcceptLexId() const
    {
        // cannot contain end node
        return -1;
    }

    void PlusNode::computeNextPos()
    {
        for (auto last : m_LastPos)
        {
            last->addNext(m_FirstPos);
        }
        m_pChild->computeNextPos();
    }

    void PlusNode::computeFirstPos()
    {
        m_pChild->computeFirstPos();
        addFirst(m_pChild->getFirstPos());
    }

    void PlusNode::computeLastPos()
    {
        m_pChild->computeLastPos();
        addLast(m_pChild->getLastPos());
    }

#ifdef TRACE_EXPREG
    void PlusNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "node PLUS" << "    First(" << m_FirstPos.toDebugIdList() << ")    Next(" << m_NextPos.toDebugIdList() << ")    Last(" << m_LastPos.toDebugIdList() << ")" << std::endl;
        m_pChild->trace(str, indent + 1);
    }
#endif

    ///////////////////////////////////////////////////////////////
    //	class LeafNode

    StarNode::StarNode(std::shared_ptr<BaseLexerTreeNode>	child)
        :m_pChild(child)
    {
    }

    void StarNode::computeNextPos()
    {
        for (auto last : m_LastPos)
        {
            last->addNext(m_FirstPos);
        }
        m_pChild->computeNextPos();
    }

    void StarNode::computeFirstPos()
    {
        m_pChild->computeFirstPos();
        addFirst(m_pChild->getFirstPos());
    }

    void StarNode::computeLastPos()
    {
        m_pChild->computeLastPos();
        addLast(m_pChild->getLastPos());
    }

    bool StarNode::nullable()
    {
        return true;
    }

    std::shared_ptr<BaseLexerTreeNode> StarNode::clone() const
    {
        return std::make_shared<StarNode>(m_pChild);
    }

    std::string StarNode::toDebugString() const
    {
        std::string debug = "(";
        debug += m_pChild->toDebugString();
        debug += ")*";
        return debug;
    }

    int StarNode::getAcceptLexId() const
    {
        // cannot contain end node
        return -1;
    }

#ifdef TRACE_EXPREG
    void StarNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "node STAR" << "    First(" << m_FirstPos.toDebugIdList() << ")    Next(" << m_NextPos.toDebugIdList() << ")    Last(" << m_LastPos.toDebugIdList() << ")" << std::endl;
        m_pChild->trace(str, indent + 1);
    }
#endif

    ///////////////////////////////////////////////////////////////
    //	class LeafNode

    LeafNode::LeafNode(CHARSET_TYPE val)
    {
        auto v = std::make_shared<MultiInterval>();
		v->addInterval(Interval(val, val));
        m_Val = v;
    }

	LeafNode::LeafNode(std::shared_ptr<Set> i)
	{
		m_Val = i;
	}

    bool LeafNode::nullable()
    {
        return false;
    }

    std::shared_ptr<BaseLexerTreeNode> LeafNode::clone() const
    {
		std::shared_ptr<Set> val = std::dynamic_pointer_cast<Set>(m_Val->clone());
        return std::make_shared<LeafNode>(val);
    }

    bool LeafNode::operator==(const CharValueNode & node) const
    {
        auto leaf = dynamic_cast<const LeafNode*>(&node);
        if (leaf)
        {
            return m_Val == leaf->m_Val;
        }
        return false;
    }

    std::string LeafNode::toCpp(const std::string& varName, bool inverse) const
    {
        return m_Val->toCpp(varName, inverse);
    }

    std::string LeafNode::toDebugString() const
    {
        return m_Val->toDebugString();
    }

    void LeafNode::computeNextPos()
    {
        // nothing to do
    }

    void LeafNode::computeFirstPos()
    {
        addFirst(this);
    }

    void LeafNode::computeLastPos()
    {
        addLast(this);
    }

    std::shared_ptr<Set> LeafNode::getValuesSet()
    {
        return m_Val;
    }

#ifdef TRACE_EXPREG
    void LeafNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "#" << getId() << " value ";
        str << m_Val->toDebugString() << "    Next(" << m_NextPos.toDebugIdList() << ")";
        str << std::endl;
    }
#endif

    ///////////////////////////////////////////////////////////////
    //	class RangeNode
    RangeNode::RangeNode()
    {
        m_Val = std::make_shared<MultiInterval>();
    }

    void RangeNode::addInterval(const Interval& i)
    {
        m_Val->addInterval(i);
    }

    void RangeNode::computeNextPos()
    {
        // nothing to do
    }

    void RangeNode::computeFirstPos()
    {
        addFirst(this);
    }

    void RangeNode::computeLastPos()
    {
        addLast(this);
    }

    std::shared_ptr<Set> RangeNode::getValuesSet()
    {
        return m_Val;
    }

    bool RangeNode::nullable()
    {
        return false;
    }

    std::shared_ptr<BaseLexerTreeNode> RangeNode::clone() const
    {
        auto p = std::make_shared<RangeNode>();
        for (const auto& inter : m_Val->getIntervals())
        {
            p->addInterval(inter);
        }
        return p;
    }

    bool RangeNode::operator==(const CharValueNode & node) const
    {
        auto range = dynamic_cast<const RangeNode*>(&node);
        if (range)
        {
            return m_Val == range->m_Val;
        }
        return false;
    }

    std::string RangeNode::toCpp(const std::string & varName, bool inverse) const
    {
        return m_Val->toCpp(varName, inverse);
    }

    std::string RangeNode::toDebugString() const
    {
        return m_Val->toDebugString();
    }

#ifdef TRACE_EXPREG
    void RangeNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "#" << getId() << " value ";
        str << m_Val->toDebugString() << "    Next(" << m_NextPos.toDebugIdList() << ")";
        str << std::endl;
    }
#endif


    ///////////////////////////////////////////////////////////////
    //	class ExceptNode

    ExceptNode::ExceptNode()
    {
        m_Val = std::make_shared<MultiAntiInterval>();
    }

    void ExceptNode::addException(const Interval&	exception)
    {
        m_Val->addInterval(exception);
    }

    void ExceptNode::computeNextPos()
    {
        // nothing to do
    }

    void ExceptNode::computeFirstPos()
    {
        addFirst(this);
    }

    void ExceptNode::computeLastPos()
    {
        addLast(this);
    }

    std::shared_ptr<Set> ExceptNode::getValuesSet()
    {
        return m_Val;
    }

    bool ExceptNode::nullable()
    {
        return false;
    }

    std::shared_ptr<BaseLexerTreeNode> ExceptNode::clone() const
    {
        auto node = std::make_shared<ExceptNode>();
        for (const auto& val : m_Val->getIntervals())
        {
            node->addException(val);
        }
        return node;
    }

    bool ExceptNode::operator==(const CharValueNode & node) const
    {
        auto except = dynamic_cast<const ExceptNode*>(&node);
        if (except)
        {
            return m_Val == except->m_Val;
        }
        return false;
    }

    std::string ExceptNode::toCpp(const std::string & varName, bool inverse) const
    {
        return m_Val->toCpp(varName, inverse);
    }

    std::string ExceptNode::toDebugString() const
    {
        return m_Val->toDebugString();
    }

#ifdef TRACE_EXPREG
    void ExceptNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "#" << getId() << " value ";
        str << m_Val->toDebugString() << "    Next(" << m_NextPos.toDebugIdList() << ")";
        str << std::endl;
    }
#endif

    ///////////////////////////////////////////////////////////////
    //	class EndNode

    EndNode::EndNode(short idLexAccepted)
    {
        m_idLexAccepted = idLexAccepted;
    }

    bool EndNode::nullable()
    {
        return false;
    }

    std::shared_ptr<BaseLexerTreeNode> EndNode::clone() const
    {
        return std::make_shared<EndNode>(m_idLexAccepted);
    }

    bool EndNode::operator==(const CharValueNode & node) const
    {
        auto end = dynamic_cast<const EndNode*>(&node);
        if (end)
        {
            return m_idLexAccepted == end->m_idLexAccepted;
        }
        return false;
    }

    std::string EndNode::toCpp(const std::string & varName, bool inverse) const
    {
        assert(false);
        return "#error // end node";
    }

    std::string EndNode::toDebugString() const
    {
        std::string debug = "accept ";
        debug += std::to_string(m_idLexAccepted);
        return debug;
    }

    int EndNode::getAcceptLexId() const
    {
        return m_idLexAccepted;
    }

    void EndNode::computeNextPos()
    {
        // nothing to do
    }

    void EndNode::computeFirstPos()
    {
        addFirst(this);
    }

    void EndNode::computeLastPos()
    {
        addLast(this);
    }

    std::shared_ptr<Set> EndNode::getValuesSet()
    {
        return EmptySet::get();
    }

#ifdef TRACE_EXPREG
    void EndNode::trace(std::ostream&     str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "#" << getId() << " node END" << std::endl;
    }
#endif

    ///////////////////////////////////////////////////////////////
    //	class EmptyNode

    bool EmptyNode::nullable()
    {
        return true;
    }

    std::shared_ptr<BaseLexerTreeNode> EmptyNode::clone() const
    {
        return std::make_shared<EmptyNode>();
    }

    bool EmptyNode::operator==(const CharValueNode & node) const
    {
        return dynamic_cast<const EmptyNode*>(&node)!=nullptr;
    }

    std::string EmptyNode::toCpp(const std::string & varName, bool inverse) const
    {
        assert(false);
        return "#error // empty node";
    }

    std::string EmptyNode::toDebugString() const
    {
        return "()";
    }

#ifdef TRACE_EXPREG
    void EmptyNode::trace(std::ostream & str, int indent)
    {
        for (int i = 0; i < indent; i++)
            str << "    ";
        str << "#" << getId() << " node EMPTY" << std::endl;
    }
#endif

    void EmptyNode::computeNextPos()
    {
        // nothing to do
    }

    void EmptyNode::computeFirstPos()
    {
    }

    void EmptyNode::computeLastPos()
    {
    }

    std::shared_ptr<Set> EmptyNode::getValuesSet()
    {
        return EmptySet::get();
    }

    ///////////////////////////////////////////////////////////////
    //	classes Set

	std::shared_ptr<MultiInterval> Set::wordSet = nullptr;
	std::shared_ptr<MultiInterval> Set::aphabeticSet = nullptr;
	std::shared_ptr<MultiInterval> Set::alphanumericSet = nullptr;
	std::shared_ptr<MultiInterval> Set::lowerSet = nullptr;
	std::shared_ptr<MultiInterval> Set::upperSet = nullptr;
	std::shared_ptr<MultiInterval> Set::digitSet = nullptr;
	std::shared_ptr<MultiInterval> Set::hexaDigitSet = nullptr;
	std::shared_ptr<MultiInterval> Set::spaceSet = nullptr;
	std::shared_ptr<MultiInterval> Set::blankSet = nullptr;
	std::shared_ptr<MultiInterval> Set::controlSet = nullptr;
	std::shared_ptr<MultiInterval> Set::printSet = nullptr;
	std::shared_ptr<MultiInterval> Set::graphSet = nullptr;
	std::shared_ptr<MultiInterval> Set::punctSet = nullptr;

	std::shared_ptr<MultiInterval> Set::getWordSet()
	{
		if (!wordSet)
		{
			wordSet = std::make_shared<MultiInterval>();
			wordSet->addInterval(Interval('0', '9'));
			wordSet->addInterval(Interval('a', 'z'));
			wordSet->addInterval(Interval('A', 'Z'));
			wordSet->addInterval(Interval('_', '_'));
		}
		return wordSet;
	}
	std::shared_ptr<MultiInterval> Set::getAlphabeticSet()
	{
		if (!aphabeticSet)
		{
			aphabeticSet = std::make_shared<MultiInterval>();
			aphabeticSet->addInterval(Interval('a', 'z'));
			aphabeticSet->addInterval(Interval('A', 'Z'));
		}
		return aphabeticSet;
	}
	std::shared_ptr<MultiInterval> Set::getAlphanumericSet()
	{
		if (!alphanumericSet)
		{
			alphanumericSet = std::make_shared<MultiInterval>();
			alphanumericSet->addInterval(Interval('0','9'));
			alphanumericSet->addInterval(Interval('a', 'z'));
			alphanumericSet->addInterval(Interval('A', 'Z'));
		}
		return alphanumericSet;
	}
	std::shared_ptr<MultiInterval> Set::getLowerSet()
	{
		if (!lowerSet)
		{
			lowerSet = std::make_shared<MultiInterval>();
			lowerSet->addInterval(Interval('a', 'z'));
		}
		return lowerSet;
	}
	std::shared_ptr<MultiInterval> Set::getUpperSet()
	{
		if (!upperSet)
		{
			upperSet = std::make_shared<MultiInterval>();
			upperSet->addInterval(Interval('A', 'Z'));
		}
		return upperSet;
	}
	std::shared_ptr<MultiInterval> Set::getDigitSet()
	{
		if (!digitSet)
		{
			digitSet = std::make_shared<MultiInterval>();
			digitSet->addInterval(Interval('0', '9'));

		}
		return digitSet;
	}
	std::shared_ptr<MultiInterval> Set::getHexaDigitSet()
	{
		if (!hexaDigitSet)
		{
			hexaDigitSet = std::make_shared<MultiInterval>();
			hexaDigitSet->addInterval(Interval('0', '9'));
			hexaDigitSet->addInterval(Interval('a', 'f'));
			hexaDigitSet->addInterval(Interval('A', 'F'));
		}
		return hexaDigitSet;
	}
	std::shared_ptr<MultiInterval> Set::getSpaceSet()
	{
		if (!spaceSet)
		{
			spaceSet = std::make_shared<MultiInterval>();
			spaceSet->addInterval(Interval(' ', ' '));
			spaceSet->addInterval(Interval('\t', '\t'));
			spaceSet->addInterval(Interval('\n', '\n'));
			spaceSet->addInterval(Interval('\v', '\v'));
			spaceSet->addInterval(Interval('\f', '\f'));
			spaceSet->addInterval(Interval('\r', '\r'));
		}
		return spaceSet;
	}
	std::shared_ptr<MultiInterval> Set::getBlankSet()
	{
		if (!blankSet)
		{
			blankSet = std::make_shared<MultiInterval>();
			blankSet->addInterval(Interval(' ', ' '));
			blankSet->addInterval(Interval('\t', '\t'));
		}
		return blankSet;
	}
	std::shared_ptr<MultiInterval> Set::getControlSet()
	{
		if (!controlSet)
		{
			controlSet = std::make_shared<MultiInterval>();
			controlSet->addInterval(Interval(0, 31));
			controlSet->addInterval(Interval(127, 127));
		}
		return controlSet;
	}
	std::shared_ptr<MultiInterval> Set::getPrintSet()
	{
		if (!printSet)
		{
			printSet = std::make_shared<MultiInterval>();
			printSet->addInterval(Interval(32, 126));
		}
		return printSet;
	}
	std::shared_ptr<MultiInterval> Set::getGraphSet()
	{
		if (!graphSet)
		{
			graphSet = std::make_shared<MultiInterval>();
			graphSet->addInterval(Interval(33, 126));
		}
		return graphSet;
	}
	std::shared_ptr<MultiInterval> Set::getPunctSet()
	{
		if (!punctSet)
		{
			punctSet = std::make_shared<MultiInterval>();
			punctSet->addInterval(Interval(33, 47));
			punctSet->addInterval(Interval(58, 64));
			punctSet->addInterval(Interval(91, 96));
			punctSet->addInterval(Interval(123, 126));
		}
		return punctSet;
	}

    std::shared_ptr<Interval> Set::convertToInterval(std::shared_ptr<Set> s)
    {
        auto *interval = dynamic_cast<const Interval*>(s.get());
        if (interval)
            return std::static_pointer_cast<Interval>(s);
        auto *multi = dynamic_cast<const MultiInterval*>(s.get());
        if (multi && multi->getIntervals().size() == 1)
        {
            const auto& i = *multi->getIntervals().begin();
            return std::make_shared<Interval>(i.getMin(), i.getMax());
        }
        return nullptr;
    }

    std::shared_ptr<EmptySet> EmptySet::emptySet=nullptr;

    std::shared_ptr<EmptySet> EmptySet::get()
    {
        if (emptySet == nullptr)
            emptySet = std::shared_ptr<EmptySet>(new EmptySet());
        return emptySet;
    }

    EmptySet::EmptySet() {}

    std::shared_ptr<Set> EmptySet::intersectWith(std::shared_ptr<Set> i) const
    {
        return get();
    }

    std::shared_ptr<Set> EmptySet::unionWith(std::shared_ptr<Set> i) const
    {
        return i;
    }

    std::shared_ptr<Set> EmptySet::substract(std::shared_ptr<Set> i) const
    {
        return get();
    }

    std::shared_ptr<Set> EmptySet::reverse() const
    {
        return WholeSet::get();
    }

    bool EmptySet::operator ==(const Set& s) const
    {
        return &s==(Set*)emptySet.get();
    }

    std::shared_ptr<Set> EmptySet::clone() const
    {
        return get();
    }

    std::string EmptySet::toCpp(const std::string & varName, bool inverse) const
    {
        assert(false);
        return "#error // empty set";
    }

    std::string EmptySet::toDebugString() const
    {
        return "()";
    }

    std::shared_ptr<WholeSet> WholeSet::wholeSet = nullptr;

    std::shared_ptr<WholeSet> WholeSet::get()
    {
        if (wholeSet == nullptr)
            wholeSet = std::shared_ptr<WholeSet>(new WholeSet());
        return wholeSet;
    }

    WholeSet::WholeSet() {}

    std::shared_ptr<Set> WholeSet::intersectWith(std::shared_ptr<Set> i) const
    {
        return i;
    }

    std::shared_ptr<Set> WholeSet::unionWith(std::shared_ptr<Set> i) const
    {
        return get();
    }

    std::shared_ptr<Set> WholeSet::substract(std::shared_ptr<Set> i) const
    {
        return i->reverse();
    }

    std::shared_ptr<Set> WholeSet::reverse() const
    {
        return EmptySet::get();
    }

    bool WholeSet::operator ==(const Set& s) const
    {
        return &s == (Set*)wholeSet.get();
    }

    std::shared_ptr<Set> WholeSet::clone() const
    {
        return get();
    }

    std::string WholeSet::toCpp(const std::string & varName, bool inverse) const
    {
        assert(false);
        return "#error // whole set";
    }

    std::string WholeSet::toDebugString() const
    {
        return ")(";
    }

    Interval::Interval(CHARSET_TYPE min, CHARSET_TYPE max)
    {
        m_min = min;
        m_max = max;
    }

    std::shared_ptr<Interval> Interval::intersectWith(const Interval &i) const
    {
        if (m_min <= i.getMax() && m_min >= i.getMin())
            return std::make_shared<Interval>(m_min, m_max > i.getMax() ? i.getMax() : m_max);
        else if (m_max <= i.getMax() && m_max >= i.getMin())
            return std::make_shared<Interval>(m_min < i.getMin() ? i.getMin() : m_min, m_max);
        else if (m_min <= i.getMin() && m_max >= i.getMax())
            return std::make_shared<Interval>(i);
        return nullptr;
    }

    std::shared_ptr<Set> Interval::unionWith(const Interval &i) const
    {
        auto intersect = intersectWith(i);
        if (intersect)
        {
            return std::make_shared<Interval>(m_min<i.getMin() ? m_min : i.getMin(), m_max>i.getMax() ? m_max : i.getMax());
        }
        else if ((m_max + 1) == i.m_min)
        {
            return std::make_shared<Interval>(m_min, i.m_max);
        }
        else if ((i.m_max + 1) == m_min)
        {
            return std::make_shared<Interval>(i.m_min, m_max);
        }
        else
        {
            std::set<Interval>  set;
            set.insert(Interval(m_min, m_max));
            set.insert(Interval(i.m_min, i.m_max));
            return std::make_shared<MultiInterval>(std::move(set));
        }
    }

    std::shared_ptr<Set> Interval::substract(const Interval & i) const
    {
        if (getMin() > i.getMax() || getMax() < i.getMin()) // disjoined sets
        {
            return std::make_shared<Interval>(*this);
        }

        if (getMin() < i.getMin())
        {
            if (getMax() > i.getMax())
            {
                std::set<Interval>  set;
                set.insert(Interval(getMin(), i.getMin() - 1));
                set.insert(Interval(i.getMax() + 1, getMax()));
                return std::make_shared<MultiInterval>(std::move(set));
            }
            else if (getMax() >= i.getMin())
            {
                return std::make_shared<Interval>(getMin(), i.getMin() - 1);
            }
        }
        else if (getMax() > i.getMax())
        {
            return std::make_shared<Interval>(i.getMax()+1, getMax());
        }
        return EmptySet::get(); // remains nothing
    }

    std::shared_ptr<Set> Interval::intersectWith(std::shared_ptr<Set> i) const
    {
        auto *interval = dynamic_cast<const Interval*>(i.get());
        if (interval)
        {
            auto intersect = intersectWith(*interval);
            if (intersect)
                return intersect;
            return EmptySet::get();
        }
        auto *multi = dynamic_cast<const MultiInterval*>(i.get());
        if (multi)
        {
            auto intersect = multi->intersectWith(*this);
            if (intersect)
                return intersect;
            return EmptySet::get();
        }
        auto *multiAnti = dynamic_cast<const MultiAntiInterval*>(i.get());
        if (multiAnti)
            return multiAnti->intersectWith(*this);
        auto *whole = dynamic_cast<const WholeSet*>(i.get());
        if (whole)
            return clone();

        return EmptySet::get();
    }

    std::shared_ptr<Set> Interval::unionWith(std::shared_ptr<Set> i) const
    {
        auto *interval = dynamic_cast<const Interval*>(i.get());
        if (interval)
            return unionWith(*interval);
        auto *multi = dynamic_cast<const MultiInterval*>(i.get());
        if (multi)
            return multi->unionWith(*this);
        auto *multiAnti = dynamic_cast<const MultiAntiInterval*>(i.get());
        if (multiAnti)
            return multiAnti->unionWith(*this);
        auto *whole = dynamic_cast<const WholeSet*>(i.get());
        if (whole)
            return WholeSet::get();

        return clone();   // result of intersect with empty set
    }

    std::shared_ptr<Set> Interval::substract(std::shared_ptr<Set> i) const
    {
        auto *interval = dynamic_cast<const Interval*>(i.get());
        if (interval)
            return substract(*interval);
        auto *multi = dynamic_cast<const MultiInterval*>(i.get());
        if (multi)
        {
            std::shared_ptr<Set> res = std::make_shared<Interval>(*this);
            for (const auto& interval : multi->getIntervals())
            {
                res = res->substract(std::make_shared<Interval>(interval));
            }
            return res;
        }
        auto *multiAnti = dynamic_cast<const MultiAntiInterval*>(i.get());
        if (multiAnti)
        {
            std::shared_ptr<Set> res = EmptySet::get();
            for (const auto& antiInterval : multiAnti->getIntervals())
            {
                auto temp = intersectWith(antiInterval);
                res = res->unionWith(temp);
            }
            return res;
        }
        auto *whole = dynamic_cast<const WholeSet*>(i.get());
        if (whole)
            return EmptySet::get();

        return clone();
    }

    std::shared_ptr<Set> Interval::reverse() const
    {
        auto anti = std::make_shared<MultiAntiInterval>();
        anti->addInterval(*this);
        return anti;
    }

    bool Interval::operator==(const Set & s) const
    {
        auto* interval = dynamic_cast<const Interval*>(&s);
        if (interval)
        {
            return getMin() == interval->getMin() && getMax() == interval->getMax();
        }
        auto* multiInterval = dynamic_cast<const MultiInterval*>(&s);
        if (multiInterval && multiInterval->getIntervals().size()==1)
        {
            return *this == *multiInterval->getIntervals().begin();
        }
        return false;
    }

    std::shared_ptr<Set> Interval::clone() const
    {
        return std::make_shared<Interval>(m_min, m_max);
    }

    std::string Interval::toCpp(const std::string & varName, bool inverse) const
    {
        std::string str;
        if (getMin() == getMax())
        {
            str = varName + (inverse ? "!=" : "==");
            str += char2stringExpr(getMax());
        }
        else
        {
            str = varName + (inverse ? "<=" : ">=");
            str += char2stringExpr(getMin());
            str += inverse ? "||" : "&&";
            str += varName + (inverse ? ">=" : "<=");
            str += char2stringExpr(getMax());
        }
        return str;
    }

    std::string Interval::toDebugString() const
    {
        std::string str;
        if (m_min == m_max)
        {
            str = char2stringExpr(m_min);
        }
        else
        {
            str += "[";
            str += char2stringExpr(m_min);
            str += "-";
            str += char2stringExpr(m_max);
            str += "]";
        }
        return str;
    }

    MultiInterval::MultiInterval() {}

    MultiInterval::MultiInterval(const MultiInterval &m)
    {
        for (const auto& interval : m.m_intervals)
            m_intervals.insert(interval);
    }

    MultiInterval::MultiInterval(std::set<Interval>&& s)
        :m_intervals(std::move(s))
    {
    }

    void MultiInterval::addInterval(const Interval &i)
    {
        for (auto iter=m_intervals.begin();iter!=m_intervals.end();iter++)
        {
            auto unionWith = (*iter).unionWith(i);
            auto interval = convertToInterval(unionWith);
            if (interval)
            {
                m_intervals.erase(iter);
                addInterval(*interval.get());
                return;
            }
        }
        m_intervals.insert(i);
    }

	void MultiInterval::addInterval(const std::set<Interval>& i)
	{
		for (const auto& inter : i)
		{
			addInterval(inter);
		}
	}

    std::shared_ptr<Set> MultiInterval::intersectWith(const Interval &i) const
    {
        auto multi = std::make_shared<MultiInterval>();
        for (const auto& interval : m_intervals)
        {
            auto intersect = interval.intersectWith(i);
            if (intersect)
                multi->addInterval(*intersect.get());
        }
        auto interval = convertToInterval(multi);
        if (interval)
            return interval;
        else if (multi->getIntervals().size())
            return multi;
        else
            return nullptr;
    }

    std::shared_ptr<Set> MultiInterval::unionWith(const Interval &i) const
    {
        auto multi = std::make_shared<MultiInterval>(*this);
        multi->addInterval(i);
        auto interval = convertToInterval(multi);
        if (interval)
            return interval;
        return multi;
    }

    std::shared_ptr<Set> MultiInterval::intersectWith(std::shared_ptr<Set> i) const
    {
        if (i == EmptySet::get())
            return i;
        else if (i == WholeSet::get())
            return clone();

        auto res = std::make_shared<MultiInterval>();
        if (auto *interval = dynamic_cast<const Interval*>(i.get()))
        {
            for (const auto& part : m_intervals)
            {
                auto intersect = interval->intersectWith(part);
                if (intersect)
                    res->addInterval(*intersect.get());
            }
        }
        else if (auto *multi = dynamic_cast<const MultiInterval*>(i.get()))
        {
            for (const auto& part1 : m_intervals)
            {
                for (const auto& part2 : multi->m_intervals)
                {
                    auto intersect = part1.intersectWith(part2);
                    if (intersect)
                        res->addInterval(*intersect.get());
                }
            }
        }
        if (auto *multiAnti = dynamic_cast<const MultiAntiInterval*>(i.get()))
        {
            for (const auto& inter : m_intervals)
            {
                std::shared_ptr<Set> rest = std::make_shared<Interval>(inter);
                for (const auto& anti : multiAnti->getIntervals())
                {
                    rest = rest->substract(std::make_shared<Interval>(anti));
                }
                res = std::static_pointer_cast<MultiInterval>(res->unionWith(rest));
            }
        }

        auto set = convertToInterval(res);
        if (set)
            return set;
        else if (res->getIntervals().size())
            return res;
        else
            return EmptySet::get();
    }

    std::shared_ptr<Set> MultiInterval::unionWith(std::shared_ptr<Set> i) const
    {
        if (auto *interval = dynamic_cast<const Interval*>(i.get()))
            return unionWith(*interval);
        else if (auto *multi = dynamic_cast<const MultiInterval*>(i.get()))
        {
            auto res = std::make_shared<MultiInterval>(*this);
            for (const auto& part : multi->m_intervals)
                res->addInterval(part);
            auto set = convertToInterval(res);
            if (set)
                return set;
            else if (res->getIntervals().size())
                return res;
        }
        else if (auto *multiAnti = dynamic_cast<const MultiAntiInterval*>(i.get()))
        {
            std::shared_ptr<Set> res = EmptySet::get();
            for (const auto& antiInterval : multiAnti->getIntervals())
            {
                auto temp = antiInterval.substract(std::make_shared<MultiInterval>(*this));
                res = res->unionWith(temp);
            }
            return res->reverse();
        }
        else if (i == WholeSet::get())
            return WholeSet::get();

        return EmptySet::get();
    }

    std::shared_ptr<Set> MultiInterval::substract(std::shared_ptr<Set> i) const
    {
        if (auto *interval = dynamic_cast<const Interval*>(i.get()))
        {
            std::shared_ptr<Set> res = EmptySet::get();
            for (const auto& inter : getIntervals())
            {
                auto temp = inter.substract(*interval);
                res = res->unionWith(temp);
            }
            return res;
        }
        else if (auto *multi = dynamic_cast<const MultiInterval*>(i.get()))
        {
            std::shared_ptr<Set> res = EmptySet::get();
            for (const auto& interval : multi->getIntervals())
            {
                std::shared_ptr<Set> res2 = EmptySet::get();
                for (const auto& inter : getIntervals())
                {
                    auto temp = inter.substract(interval);
                    res2 = res2->unionWith(temp);
                }
                if (res != EmptySet::get())
                    res = res->intersectWith(res2);
                else
                    res = res2;
            }
            return res;
        }
        else if (auto *multiAnti = dynamic_cast<const MultiAntiInterval*>(i.get()))
        {
            std::shared_ptr<Set> res = EmptySet::get();
            for (const auto& antiInterval : multiAnti->getIntervals())
            {
                auto temp = intersectWith(antiInterval);
                res = res->unionWith(temp);
            }
            return res;
        }
        else if (i == WholeSet::get())
            return EmptySet::get();

        return clone();
    }

    std::shared_ptr<Set> MultiInterval::reverse() const
    {
        auto anti = std::make_shared<MultiAntiInterval>();
        for (const auto& interval : getIntervals())
        {
            anti->addInterval(interval);
        }
        return anti;
    }

    bool MultiInterval::operator==(const Set & s) const
    {
        if (auto* multiInterval = dynamic_cast<const MultiInterval*>(&s))
        {
            if (multiInterval && getIntervals().size() != multiInterval->getIntervals().size())
                return false;
            auto iter1 = m_intervals.begin();
            auto iter2 = multiInterval->m_intervals.begin();
            for (
                ; iter1 != m_intervals.end()
                ; iter1++, iter2++)
            {
                if (!(*iter1 == *iter2))
                    return false;
            }
            return true;
        }
        else if (auto* interval = dynamic_cast<const Interval*>(&s))
        {
            if (getIntervals().size() != 1)
                return false;
            return *m_intervals.begin() == *interval;
        }
        return false;
    }

    std::shared_ptr<Set> MultiInterval::clone() const
    {
        return std::make_shared<MultiInterval>(*this);
    }

    std::string MultiInterval::toCpp(const std::string & varName, bool inverse) const
    {
        int cpt = 0;
        std::string str;
        if (inverse)
            str += "!(";
        for (const auto& interval : getIntervals())
        {
            if (cpt)
                str += "||";
            str += "(";
            str += interval.toCpp(varName, inverse);
            str += ")";
            cpt++;
        }
        if (inverse)
            str += ")";
        return str;
    }

    std::string MultiInterval::toDebugString() const
    {
		if (m_intervals.size() == 1)
		{
			return (*m_intervals.begin()).toDebugString();
		}
        std::string str="{";
        int cpt = 0;
        for (const auto& inter : m_intervals)
        {
            if (cpt>0)
                str += ",";
            str += inter.toDebugString();
            cpt++;
        }
        str += "}";
        return str;
    }

    MultiAntiInterval::MultiAntiInterval(){}

    MultiAntiInterval::MultiAntiInterval(const MultiAntiInterval &m)
        :m_intervals(m.m_intervals)
    {
    }

    MultiAntiInterval::MultiAntiInterval(MultiAntiInterval &&m)
        :m_intervals(std::move(m.m_intervals))
    {
    }

    void MultiAntiInterval::addInterval(const Interval &i)
    {
        for (auto iter = m_intervals.begin(); iter != m_intervals.end();iter++)
        {
            auto unionWith = (*iter).unionWith(i);
            auto i = convertToInterval(unionWith);
            if (i)
            {
                m_intervals.erase(iter);
                addInterval(*i.get());
                return;
            }
        }
        m_intervals.insert(i);
    }

    std::shared_ptr<Set> MultiAntiInterval::intersectWith(const Interval &i) const
    {
        std::shared_ptr<Set> res = std::make_shared<Interval>(i);
        for (const auto& antiInterval : m_intervals)
        {
            res = res->substract(std::make_shared<Interval>(antiInterval));
        }
        return res;
    }

    std::shared_ptr<Set> MultiAntiInterval::unionWith(const Interval &i) const
    {
        std::shared_ptr<Set> rest = EmptySet::get();
        for (const auto& antiInterval : m_intervals)
        {
            auto res = antiInterval.substract(i);
            rest = rest->unionWith(res);
        }
        auto res = std::make_shared<MultiAntiInterval>();
        auto *interval = dynamic_cast<const Interval*>(rest.get());
        if (interval)
        {
            res->addInterval(*interval);
            return res;
        }
        auto *multi = dynamic_cast<const MultiInterval*>(rest.get());
        if (multi)
        {
            for (const auto& interval : multi->getIntervals())
            {
                res->addInterval(interval);
            }
            return res;
        }
        return res;
    }

    std::shared_ptr<Set> MultiAntiInterval::intersectWith(std::shared_ptr<Set> i) const
    {
        if (auto *interval = dynamic_cast<const Interval*>(i.get()))
        {
            return intersectWith(*interval);
        }
        else if (auto *multi = dynamic_cast<const MultiInterval*>(i.get()))
        {
            std::shared_ptr<Set> res = std::make_shared<MultiAntiInterval>(*this);
            for (const auto& interval : multi->getIntervals())
            {
                res = res->intersectWith(std::make_shared<Interval>(interval));
            }
            return res;
        }
        else if (auto *antiMulti = dynamic_cast<const MultiAntiInterval*>(i.get()))
        {
            std::shared_ptr<Set> res = std::make_shared<MultiAntiInterval>(*this);
            for (const auto& interval : antiMulti->getIntervals())
            {
                res = res->unionWith(std::make_shared<Interval>(interval));
            }
            return res;
        }
        else if (i == WholeSet::get())
            return i;

        return EmptySet::get(); // should never happen
    }

    std::shared_ptr<Set> MultiAntiInterval::unionWith(std::shared_ptr<Set> i) const
    {
        if (auto *interval = dynamic_cast<const Interval*>(i.get()))
        {
            return unionWith(*interval);
        }
        else if (auto *multi = dynamic_cast<const MultiInterval*>(i.get()))
        {
            std::shared_ptr<Set> res = std::make_shared<MultiAntiInterval>(*this);
            for (const auto& interval : multi->getIntervals())
            {
                res = res->unionWith(std::make_shared<Interval>(interval));
            }
            return res;
        }
        else if (auto *antiMulti = dynamic_cast<const MultiAntiInterval*>(i.get()))
        {
            std::shared_ptr<Set> res = std::make_shared<MultiAntiInterval>(*this);
            for (const auto& interval : antiMulti->getIntervals())
            {
                res = res->intersectWith(std::make_shared<Interval>(interval));
            }
            return res;
        }
        else if (i == WholeSet::get())
            return WholeSet::get();

        return clone();
    }

    std::shared_ptr<Set> MultiAntiInterval::substract(std::shared_ptr<Set> i) const
    {
        if (auto *interval = dynamic_cast<const Interval*>(i.get()))
        {
            std::shared_ptr<Set> res = i;
            for (const auto& inter : getIntervals())
            {
                res = res->unionWith(std::make_shared<Interval>(inter));
            }
            return res->reverse();
        }
        else if (auto *multi = dynamic_cast<const MultiInterval*>(i.get()))
        {
            std::shared_ptr<Set> res = i;
            for (const auto& inter : getIntervals())
            {
                res = res->unionWith(std::make_shared<Interval>(inter));
            }
            return res->reverse();
        }
        else if (auto *multiAnti = dynamic_cast<const MultiAntiInterval*>(i.get()))
        {
            return multiAnti->reverse()->substract(this->reverse());
        }
        else if (i == WholeSet::get())
            return EmptySet::get();

        return clone();
    }

    std::shared_ptr<Set> MultiAntiInterval::reverse() const
    {
        auto multi = std::make_shared<MultiInterval>();
        for (const auto& interval : getIntervals())
        {
            multi->addInterval(interval);
        }
        return multi;
    }

    bool MultiAntiInterval::operator==(const Set & s) const
    {
        if (auto* interval = dynamic_cast<const MultiAntiInterval*>(&s))
        {
            if (getIntervals().size() != interval->getIntervals().size())
                return false;

            auto iter1 = m_intervals.begin();
            auto iter2 = interval->m_intervals.begin();
            for (
                ; iter1 != m_intervals.end()
                ; iter1++, iter2++)
            {
                if (!(*iter1 == *iter2))
                    return false;
            }
            return true;
        }
        return false;
    }

    std::shared_ptr<Set> MultiAntiInterval::clone() const
    {
        return std::make_shared<MultiAntiInterval>(*this);
    }

    std::string MultiAntiInterval::toCpp(const std::string & varName, bool inverse) const
    {
        int cpt = 0;
        std::string str;
        if (!inverse)
            str += "!(";
        for (const auto& interval : getIntervals())
        {
            if (cpt)
                str += "||";
            str += "(";
            str += interval.toCpp(varName, inverse);
            str += ")";
            cpt++;
        }
        if (!inverse)
            str += ")";
        return str;
    }

    std::string MultiAntiInterval::toDebugString() const
    {
        std::string str = "}";
        int cpt = 0;
        for (const auto& inter : m_intervals)
        {
            if (cpt>0)
                str += ",";
            str += inter.toDebugString();
            cpt++;
        }
        str += "{";
        return str;
    }

    ///////////////////////////////////////////////////////////////
    //	class PositionSet

    class PositionSetBreaker
    {
        CharValueNode*    toIgnore;   // current value
        std::vector<std::pair<PositionSet, PositionSet>>    result;     // pair of (position set, expression)

        const PositionSet& set2break;
    public:
        PositionSetBreaker(const PositionSet& set)
            :set2break(set)
        {}
        std::vector<std::pair<PositionSet, PositionSet>> run()
        {
            result.clear();
            for (auto *pos : set2break)
            {
                if (pos->getValuesSet() == EmptySet::get())
                    continue;
                toIgnore = pos;
                searchIntersect(pos, pos, set2break.begin());
            }
            // optimize the last result element (remove a condition if possible)
            std::shared_ptr<Set> lastSet;
            std::shared_ptr<Set> firstSets = EmptySet::get();
            int cpt = 0;
            for (auto iter = result.rbegin()
                ; iter != result.rend()
                ; iter++)
            {
                if (cpt)
                {
                    firstSets = firstSets->unionWith((*iter).second.intersectAll());
                }
                else
                {
                    lastSet = (*iter).second.intersectAll();
                }
                cpt++;
            }
            if (lastSet && *lastSet->reverse() == *firstSets)
            {
                // transform the last element in an "else"
                auto& last = *result.rbegin();
                last.second.clear();
            }
            return result;
        }

    private:
        void searchIntersect( const PositionSet& set, const PositionSet& expr, PositionSet::iterator next )
        {
            while (next!=set2break.end() && (toIgnore==*next || (*next)->getValuesSet()==EmptySet::get()))
                next++;
            auto intersectSet = set.intersectAll();
            auto iter2 = next;
            while (iter2 != set2break.end())
            {
                auto pos = *iter2;
                iter2++;
                const auto set_and_pos = set.with(pos);
                auto intersectTotal = intersectSet->intersectWith(pos->getValuesSet());
                if (intersectTotal == EmptySet::get())
                    continue;
                auto set_isIn_pos = (*intersectSet == *intersectTotal.get());
                auto pos_isIn_set = (*pos->getValuesSet() == *intersectTotal.get());

                // 1 - treat the intersection part
                searchIntersect(set_and_pos, pos_isIn_set?pos: (set_isIn_pos?expr: set_and_pos), iter2);

                // 2 - treat the parts of the 2 sets that are not common
                if (set_isIn_pos)
                {
                    if (!pos_isIn_set)
                        searchIntersect(set_and_pos.removeEachIn(intersectTotal), pos, iter2);
                    return;
                }
                else if (!pos_isIn_set)
                {
                    searchIntersect(set_and_pos.removeEachIn(intersectTotal), pos, iter2);
                }
                searchIntersect(set_and_pos, pos, iter2);
                return;
            }
            for (const auto& pair : result)
            {
                auto& existingSet = pair.first;
                if (set == existingSet)
                    return;
            }
            if (expr.hasOnlySingleChar())
                result.insert(result.cbegin(), std::move(std::pair<PositionSet, PositionSet>(set, expr)));
            else
                result.emplace_back(set, expr);
        }
    };

    std::vector<std::pair<PositionSet, PositionSet>> PositionSet::BreakByPositionSet()
    {
        PositionSetBreaker  breaker(*this);
        return breaker.run();
    }

    int PositionSet::getAcceptLexId() const
    {
        auto minLex = -1;
        for (auto *val : *this)
        {
            auto lex = val->getAcceptLexId();
            if ((minLex == -1 || lex < minLex) && lex>=0)
                minLex = lex;
        }
        return minLex;
    }

    std::string PositionSet::toDebugString() const
    {
        std::string debug;
        for (auto *val : *this)
        {
            if (debug.size())
                debug += "&&";
            debug += val->toDebugString();
        }
        return debug;
    }

    std::string PositionSet::toDebugIdList() const
    {
        std::string debug;
        for (auto *val : *this)
        {
            debug += "#";
            debug += std::to_string(val->getId());
            debug += " ";
        }
        return debug;
    }

    std::shared_ptr<Set> PositionSet::intersectAll() const
    {
        std::shared_ptr<Set> set;
        for (auto *val : *this)
        {
            if (set)
                set = set->intersectWith(val->getValuesSet());
            else
                set = val->getValuesSet();
        }
        if (set)
            return set;
        return EmptySet::get();
    }

    PositionSet PositionSet::removeEachIn(std::shared_ptr<Set> set) const
    {
        PositionSet ret;
        for (auto *val : *this)
        {
            if (val->getValuesSet()->substract(set) == EmptySet::get())
                continue;
            ret.insert(val);
        }
        return ret;
    }

    bool PositionSet::operator==(const PositionSet & s) const
    {
        if (size() != s.size())
            return false;
        auto iter1 = begin();
        auto iter2 = s.begin();
        for (; iter1 != end(); iter1++, iter2++)
        {
            if (*iter1 != *iter2)
                return false;
        }
        return true;
    }

    PositionSet & PositionSet::operator=(PositionSet&& set)
    {
        *(std::set<CharValueNode*>*)this = std::move(set);
        return *this;
    }

    bool PositionSet::hasOnlySingleChar() const
    {
        auto set = intersectAll();
        if (auto interval = std::dynamic_pointer_cast<Interval>(set))
        {
            return interval->getMin() == interval->getMax();
        }
        if (auto multiInterval = std::dynamic_pointer_cast<MultiInterval>(set))
        {
            if (multiInterval->getIntervals().size() != 1)
                return false;
            auto interval = *multiInterval->getIntervals().begin();
            return interval.getMin() == interval.getMax();
        }
        return false;
    }

    CharValueNode::CharValueNode()
    {
        id = nextId;
        nextId++;
    }

    int CharValueNode::getAcceptLexId() const
    {
        // by default:
        return -1;
    }
    int CharValueNode::getId() const
    {
        return id;
    }
    int CharValueNode::nextId = 0;
}
