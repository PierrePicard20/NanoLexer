#pragma once

#include <set>
#include <memory>
#include <vector>
#include <map>
#include <string>

namespace RegularExpression
{
    class Interval;
    class PositionSet;
    class CharValueNode;
}

//#define TRACE_EXPREG

namespace std
{
    template <>
    class less<::RegularExpression::Interval>
    {
    public:
        bool operator()(const ::RegularExpression::Interval &lhs, const ::RegularExpression::Interval &rhs) const;
    };

    template <>
    class hash<::RegularExpression::PositionSet>
    {
        static std::hash<::RegularExpression::CharValueNode*>    hasher;
    public:
        std::size_t operator()(const ::RegularExpression::PositionSet& s) const;
    };

    template <>
    class less<::RegularExpression::CharValueNode*>
    {
    public:
        bool operator()(const ::RegularExpression::CharValueNode* lhs, const ::RegularExpression::CharValueNode* rhs) const;
    };
}

namespace RegularExpression
{
    using CHARSET_TYPE=unsigned char;

    std::string char2stringExpr(unsigned char v);

    class CharValueNode;
    class ExpressionNode;
    class Set;
    class PositionSet : public std::set<CharValueNode*>
    {
    public:
        PositionSet() {}
        PositionSet(const PositionSet &s);
        PositionSet(PositionSet &&s) noexcept :std::set<CharValueNode*>(std::move(s)) {}
        PositionSet(CharValueNode *v) { insert(v); }

        std::vector<std::pair<PositionSet, PositionSet>> BreakByPositionSet();
        int getAcceptLexId() const;
        std::string toDebugString() const;
        std::shared_ptr<Set> intersectAll() const;
        PositionSet removeEachIn(std::shared_ptr<Set> set) const;
        bool operator ==(const PositionSet& s) const;
        PositionSet& operator=(PositionSet&& set);
        bool hasOnlySingleChar() const;
        PositionSet with(CharValueNode *v) const { PositionSet s = *this; s.insert(v); return s; }

        std::string toDebugIdList() const;
    };


    class Interval;
	class MultiInterval;
    class Set
    {
		static std::shared_ptr<MultiInterval> wordSet;
		static std::shared_ptr<MultiInterval> aphabeticSet;
		static std::shared_ptr<MultiInterval> alphanumericSet;
		static std::shared_ptr<MultiInterval> lowerSet;
		static std::shared_ptr<MultiInterval> upperSet;
		static std::shared_ptr<MultiInterval> digitSet;
		static std::shared_ptr<MultiInterval> hexaDigitSet;
		static std::shared_ptr<MultiInterval> spaceSet;
		static std::shared_ptr<MultiInterval> blankSet;
		static std::shared_ptr<MultiInterval> controlSet;
		static std::shared_ptr<MultiInterval> printSet;
		static std::shared_ptr<MultiInterval> graphSet;
		static std::shared_ptr<MultiInterval> punctSet;

    public:
        virtual std::shared_ptr<Set> intersectWith(std::shared_ptr<Set> i) const = 0;
        virtual std::shared_ptr<Set> unionWith(std::shared_ptr<Set> i) const = 0;
        virtual std::shared_ptr<Set> substract(std::shared_ptr<Set> i) const = 0;
        virtual std::shared_ptr<Set> reverse() const = 0;
        virtual bool operator ==(const Set& s) const = 0;
        virtual std::shared_ptr<Set> clone() const = 0;
        virtual std::string toCpp(const std::string& varName, bool inverse) const = 0;
        virtual std::string toDebugString() const = 0;
        
        static std::shared_ptr<Interval> convertToInterval(std::shared_ptr<Set> s);

		static std::shared_ptr<MultiInterval> getWordSet();			//				\w: word; \W: all but a word
		static std::shared_ptr<MultiInterval> getAlphabeticSet();	// [:alpha:]
		static std::shared_ptr<MultiInterval> getAlphanumericSet();	// [:alnum:]
		static std::shared_ptr<MultiInterval> getLowerSet();		// [:lower:]
		static std::shared_ptr<MultiInterval> getUpperSet();		// [:upper:]
		static std::shared_ptr<MultiInterval> getDigitSet();		// [:digit:] or \d: a digit;\D: non-digit
		static std::shared_ptr<MultiInterval> getHexaDigitSet();	// [:xdigit:]
		static std::shared_ptr<MultiInterval> getSpaceSet();		// [:space:] or \s: spaces; \S: all but spaces
		static std::shared_ptr<MultiInterval> getBlankSet();		// [:blank:]
		static std::shared_ptr<MultiInterval> getControlSet();		// [:cntrl:]
		static std::shared_ptr<MultiInterval> getPrintSet();		// [:print:]
		static std::shared_ptr<MultiInterval> getGraphSet();		// [:graph:]
		static std::shared_ptr<MultiInterval> getPunctSet();		// [:punct:]
	};


    class EmptySet : public Set
    {
        static std::shared_ptr<EmptySet> emptySet;
        EmptySet();
    public:
        static std::shared_ptr<EmptySet> get();
        virtual std::shared_ptr<Set> intersectWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> unionWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> substract(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> reverse() const override;
        virtual bool operator ==(const Set& s) const override;
        virtual std::shared_ptr<Set> clone() const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;
    };

    class WholeSet : public Set
    {
        static std::shared_ptr<WholeSet> wholeSet;
        WholeSet();
    public:
        static std::shared_ptr<WholeSet> get();
        virtual std::shared_ptr<Set> intersectWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> unionWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> substract(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> reverse() const override;
        virtual bool operator ==(const Set& s) const override;
        virtual std::shared_ptr<Set> clone() const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;
    };

    class Interval : public Set
    {
        CHARSET_TYPE m_min, m_max;

    public:
        Interval(CHARSET_TYPE min, CHARSET_TYPE max);

        inline CHARSET_TYPE getMin() const { return m_min; }
        inline CHARSET_TYPE getMax() const { return m_max; }
        std::shared_ptr<Interval> intersectWith(const Interval &i) const;
        std::shared_ptr<Set> unionWith(const Interval &i) const;
        std::shared_ptr<Set> substract(const Interval &i) const;

        virtual std::shared_ptr<Set> intersectWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> unionWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> substract(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> reverse() const override;
        virtual bool operator ==(const Set& s) const override;
        virtual std::shared_ptr<Set> clone() const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;
    };

    class MultiInterval : public Set
    {
        std::set<Interval>   m_intervals;

    public:
        MultiInterval();
        MultiInterval(const MultiInterval &m);
        MultiInterval(std::set<Interval>&& s);
        inline const std::set<Interval>& getIntervals() const { return m_intervals; }
        void addInterval(const Interval &i);
		void addInterval(const std::set<Interval>& i);
        std::shared_ptr<Set> intersectWith(const Interval &i) const;
        std::shared_ptr<Set> unionWith(const Interval &i) const;

        virtual std::shared_ptr<Set> intersectWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> unionWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> substract(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> reverse() const override;
        virtual bool operator ==(const Set& s) const override;
        virtual std::shared_ptr<Set> clone() const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;
    };

    class MultiAntiInterval : public Set
    {
        std::set<Interval>   m_intervals;

    public:
        MultiAntiInterval();
        MultiAntiInterval(const MultiAntiInterval &m);
        MultiAntiInterval(MultiAntiInterval &&m);
        inline const std::set<Interval>& getIntervals() const { return m_intervals; }
        void addInterval(const Interval &i);
        std::shared_ptr<Set> intersectWith(const Interval &i) const;
        std::shared_ptr<Set> unionWith(const Interval &i) const;

        virtual std::shared_ptr<Set> intersectWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> unionWith(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> substract(std::shared_ptr<Set> i) const override;
        virtual std::shared_ptr<Set> reverse() const override;
        virtual bool operator ==(const Set& s) const override;
        virtual std::shared_ptr<Set> clone() const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////

    class BaseLexerTreeNode
    {
    protected:
        PositionSet	    m_FirstPos;
        PositionSet	    m_LastPos;
        PositionSet	    m_NextPos;

    public:
        inline const PositionSet& getFirstPos() const { return m_FirstPos; }
        inline const PositionSet& getLastPos() const { return m_LastPos; }
        inline const PositionSet& getNextPos() const { return m_NextPos; }

        inline void addFirst(CharValueNode* node) { m_FirstPos.insert(node); }
        inline void addFirst(const PositionSet& nodes) { m_FirstPos.insert(nodes.begin(), nodes.end()); }
        inline void addLast(CharValueNode* node) { m_LastPos.insert(node); }
        inline void addLast(const PositionSet& nodes) { m_LastPos.insert(nodes.begin(), nodes.end()); }
        inline void addNext(const PositionSet& nodes) { m_NextPos.insert(nodes.begin(), nodes.end()); }

        virtual void computeNextPos() = 0;
        virtual void computeFirstPos() = 0;
        virtual void computeLastPos() = 0;

        virtual bool nullable() = 0;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const = 0;
        virtual std::string toDebugString() const = 0;
        virtual int getAcceptLexId() const = 0;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) = 0;
#endif
    };

    class ConcatNode : public BaseLexerTreeNode
    {
        std::vector<std::shared_ptr<BaseLexerTreeNode>>	m_Children;

    public:
        ConcatNode();
        ConcatNode(std::vector<std::shared_ptr<BaseLexerTreeNode>>&& children);

        void addChild(std::shared_ptr<BaseLexerTreeNode>	child);

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual std::string toDebugString() const override;
        virtual int getAcceptLexId() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class OrNode : public BaseLexerTreeNode
    {
        std::vector<std::shared_ptr<BaseLexerTreeNode>>	m_Children;

    public:
        OrNode();
        OrNode(std::vector<std::shared_ptr<BaseLexerTreeNode>>&& children);

        void addChild(std::shared_ptr<BaseLexerTreeNode>	child);

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual std::string toDebugString() const override;
        virtual int getAcceptLexId() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class OptionNode : public BaseLexerTreeNode
    {
        std::shared_ptr<BaseLexerTreeNode>	m_pChild;

    public:
        OptionNode(std::shared_ptr<BaseLexerTreeNode>	child);

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual std::string toDebugString() const override;
        virtual int getAcceptLexId() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class PlusNode : public BaseLexerTreeNode
    {
        std::shared_ptr<BaseLexerTreeNode>	m_pChild;

    public:
        PlusNode(std::shared_ptr<BaseLexerTreeNode>	child);

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual std::string toDebugString() const override;
        virtual int getAcceptLexId() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class StarNode : public BaseLexerTreeNode
    {
        std::shared_ptr<BaseLexerTreeNode>	m_pChild;

    public:
        StarNode(std::shared_ptr<BaseLexerTreeNode>	child);

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual std::string toDebugString() const override;
        virtual int getAcceptLexId() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class CharValueNode : public BaseLexerTreeNode
    {
        int id;
        static int nextId;
    public:
        CharValueNode();
        virtual std::shared_ptr<Set> getValuesSet() = 0;
        virtual bool operator==(const CharValueNode& node) const = 0;
        virtual std::string toCpp(const std::string& varName, bool inverse) const = 0;
        virtual int getAcceptLexId() const override;
        int getId() const;
    };

    class LeafNode : public CharValueNode
    {
        std::shared_ptr<Set>		m_Val;
    public:
        LeafNode(CHARSET_TYPE val);
		LeafNode(std::shared_ptr<Set> i);

        inline std::shared_ptr<Set> getVal() const { return m_Val; }

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual std::shared_ptr<Set> getValuesSet() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual bool operator==(const CharValueNode& node) const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class RangeNode : public CharValueNode
    {
        std::shared_ptr<MultiInterval>    m_Val;
    public:
        RangeNode();
        void addInterval(const Interval& i);

        inline std::shared_ptr<MultiInterval> getVal() const { return m_Val; }

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual std::shared_ptr<Set> getValuesSet() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual bool operator==(const CharValueNode& node) const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class ExceptNode : public CharValueNode
    {
        std::shared_ptr<MultiAntiInterval>   m_Val;

    public:
        ExceptNode();

        void addException(const Interval&	exception);

        inline std::shared_ptr<MultiAntiInterval> getVal() const { return m_Val; }

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual std::shared_ptr<Set> getValuesSet() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual bool operator==(const CharValueNode& node) const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class EndNode : public CharValueNode
    {
        short		m_idLexAccepted;

    public:
        EndNode(short idLexAccepted);

        inline short getIdLexAccepted() const { return m_idLexAccepted; }

        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual std::shared_ptr<Set> getValuesSet() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual bool operator==(const CharValueNode& node) const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;
        virtual int getAcceptLexId() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };

    class EmptyNode : public CharValueNode
    {
    public:
        virtual void computeNextPos() override;
        virtual void computeFirstPos() override;
        virtual void computeLastPos() override;

        virtual std::shared_ptr<Set> getValuesSet() override;

        virtual bool nullable() override;
        virtual std::shared_ptr<BaseLexerTreeNode> clone() const override;
        virtual bool operator==(const CharValueNode& node) const override;
        virtual std::string toCpp(const std::string& varName, bool inverse) const override;
        virtual std::string toDebugString() const override;

#ifdef TRACE_EXPREG
        virtual void trace(std::ostream&     str, int indent) override;
#endif
    };
}
