#pragma once

#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <vector>
#include <sstream>

#define T_EXPRTREE	1

//#define TRACE_EXPREG
#include "LexerTreeNode.h"

namespace RegularExpression
{
    class State;
}

namespace std
{
    template <>
    class less<::RegularExpression::State>
    {
    public:
        bool operator()(const ::RegularExpression::State &lhs, const ::RegularExpression::State &rhs) const;
    };
}

namespace RegularExpression
{
    extern std::fstream		f_Unit_Lexer;

    typedef std::unordered_set<CHARSET_TYPE>   SET_CHAR;

    class LexerContext;
    class State
    {
        PositionSet      m_positions;
        short       m_ident;
        std::vector<std::pair<PositionSet, State*>>    m_transitions;     // vector expression, state
        short       m_idLexAccepted;      // -1 if none
        short       m_nbRef;

        void printAction(bool isMainContext, std::ostringstream& scode, const State* nextState, const std::set<int>& pops, const std::map<int, LexerContext*>& pushCtx, const std::map<int, std::string>& id2OnMatchCode, bool withOnFailure) const;
    public:
        State(const PositionSet &a, short id, short acceptLex);
        State(const State&) = delete;
        State(State&& s) noexcept;

        bool operator==(const State& s) const;

        void    ComputeNextStates(std::map<PositionSet, State>&   m_mapState, std::vector<State*>&   listState);
        bool    removeRedundantTransitions();
        bool    replace(const State* oldState, State* newState);
        std::string toDebugString(const std::set<int>& pops) const;

        inline short GetId() const { return m_ident; }
        inline void setIdLexAccepted(short idLex) { if (m_idLexAccepted == -1 || idLex < m_idLexAccepted) m_idLexAccepted = idLex; }
        inline short getIdLexAccepted() const { return m_idLexAccepted; }
        inline void incrRefCount() { m_nbRef++; }
        inline bool isReferenced() const { return m_nbRef > 0; }

        inline const std::vector<std::pair<PositionSet, State*>>& getTransitions() const { return m_transitions; }
        inline const PositionSet& getPositions() const { return m_positions; }

        bool HasNextStateAsAcceptState() const;
        bool IsRecursiveState() const;
        std::string toCpp(const std::set<int>& pops, const std::map<int, LexerContext*>& pushCtx, const std::map<int, std::string>& id2OnMatchCode, bool withNoread, bool withOnFailure) const;
    };

    class LexerContext
    {
        std::unique_ptr<BaseLexerTreeNode>	m_phead;
        std::map<PositionSet, State>        m_mapState;
        std::vector<State*>                 m_states;

        std::vector<short>			m_AcceptState;		// array of accept state. lenght of array = number of states.
        // if m_AcceptState[i]=0 => the state i is not an accept state
        // if m_AcceptState[i]=n => the state i is an accept state for the lexeme of id n.

        std::string	    m_name;
        int             m_lexemeCount;
        std::set<int>   m_pops;     // only for sub contexts, contains lexeme ids
        std::map<int, LexerContext*>    m_mapPush;  // lexeme id => context to be pushed
        std::string     onMatchCode;

    public:
        LexerContext(const std::string& name, int lexemeCount, const std::set<int>& pops);
        LexerContext(const LexerContext& ctx) = delete;
        LexerContext(LexerContext&& cts) noexcept;

        inline const std::vector<State*>& getStates() const { return m_states; }
        inline int getLexemeCount() const { return m_lexemeCount; }
        inline const std::string& getName() const { return m_name; }
        inline void addPushContext(int idLex, LexerContext* ctx) { m_mapPush.emplace(idLex, ctx); }

        void setExpRegList(std::vector<std::shared_ptr<BaseLexerTreeNode>>&& list);
        void addExpression(std::shared_ptr<BaseLexerTreeNode> expr);

        void ComputeLexer();

        BaseLexerTreeNode *GetExprTree();
        const std::map<PositionSet, State>& getMapState() const;
        std::string declareToCpp() const;
        std::string bodyToCpp(const std::string& onFailure, const std::map<int, std::string>& id2OnMatchCode) const;
        std::string getOnMatchCode();
        std::string getDebugString(const std::set<int>& popIds) const;
    };
}
