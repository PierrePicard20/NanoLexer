#pragma warning(disable:4503)

#include <string>
#include <fstream>
#include <memory.h>
#include <stdio.h>
#include <cassert>

#include "LexerBuilder.h"

#ifdef TRACE_EXPREG
std::fstream	trace_expreg;
#endif

#define INDENT		for (int z=0;z<indent;z++) {scode << "    ";}

namespace std
{
    bool less<::RegularExpression::State>::operator()(const ::RegularExpression::State &lhs, const ::RegularExpression::State &rhs) const
    {
        return lhs.GetId() < rhs.GetId();
    }
}

namespace RegularExpression
{
    //////////////////////////////////////////////////////////////////////////
    //  State

    State::State(const PositionSet &a, short id, short acceptLex)
        :m_positions(a)
    {
        m_ident = id;
        m_idLexAccepted = acceptLex;
        m_nbRef = 0;
    }

    State::State(State&& s) noexcept
        :m_positions(std::move(s.m_positions))
        ,m_transitions(std::move(s.m_transitions))
    {
        m_ident = s.m_ident;
        m_idLexAccepted = s.m_idLexAccepted;
        m_nbRef = s.m_nbRef;
    }

    bool State::operator==(const State & s) const
    {
        if (m_idLexAccepted != s.m_idLexAccepted)
            return false;
        if (m_transitions.size() != s.m_transitions.size())
            return false;
        auto iter1 = m_transitions.begin();
        auto iter2 = s.m_transitions.begin();
        for (;iter1 != m_transitions.end();iter1++,iter2++)
        {
            if (iter1->first.intersectAll() != iter2->first.intersectAll())
                return false;
            if (iter1->second != iter2->second)
                return false;
        }
        return true;
    }

    void    State::ComputeNextStates(std::map<PositionSet, State>&   mapState, std::vector<State*>&   listState)
    {
        auto expressions = m_positions.BreakByPositionSet();
        for (const auto& pair : expressions)
        {
            auto& positionSet = pair.first;
            PositionSet next;
            for (auto pos : positionSet)
            {
                auto temp = pos->getNextPos();
                next.insert(temp.begin(), temp.end());
            }

            if (next.size() == 0)
                continue;

            State *newState = nullptr;
            auto iter = mapState.find(next);
            if (iter == mapState.end())
            {
                int nextId = (int)mapState.size();
                auto iter = mapState.emplace(next, State(next,nextId,next.getAcceptLexId()));
                assert(iter.second);
                newState = &iter.first->second;
                listState.push_back(newState);
            }
            else
                newState = &iter->second;

            // if it's not a duplicate transition ... 
            bool alreadyExist = false;
            for (auto& transition : m_transitions)
            {
                auto exprDisplay = transition.first;
                auto state = transition.second;
                if (state == newState && exprDisplay == pair.second)
                {
                    alreadyExist = true;
                    break;
                }
            }
            if (!alreadyExist)
                // ... we save the new transition
                m_transitions.emplace_back(pair.second, newState);
        }
    }

    bool State::removeRedundantTransitions()
    {
        bool changed = false;
        for (auto iter = m_transitions.begin(); iter != m_transitions.end();)
        {
            const auto& expr1 = iter->first;
            const State* state1 = iter->second;
            auto next = iter;
            next++;
            bool toRemove = false;
            for (; next != m_transitions.end(); next++)
            {
                const auto& expr2 = next->first;
                const State* state2 = next->second;
                if (state1 == state2)
                {
                    if (expr1.intersectAll()->substract(expr2.intersectAll()) == EmptySet::get())
                    {
                        toRemove = true;
                        changed = true;
                        break;
                    }
                }
            }
            if (toRemove)
                iter = m_transitions.erase(iter);
            else
                iter++;
        }
        return changed;
    }

    bool State::replace(const State * oldState, State * newState)
    {
        bool changed = false;
        for (auto& transition : m_transitions)
        {
            if (transition.second == oldState)
            {
                transition.second = newState;
                changed = true;
            }
        }
        return changed;
    }

    std::string State::toDebugString(const std::set<int>& pops) const
    {
        auto isMainContext = (pops.size() == 0);
        std::string debug = "s";
        debug += std::to_string(GetId());
        debug += ":";
        for (const auto& pair : getTransitions())
        {
            auto exprNode = pair.first;
            auto *nextState = pair.second;
            auto exprStr = exprNode.toDebugString();
            debug += exprStr.size()?exprStr:"else";
            if (nextState->getTransitions().size())
            {
                debug += "->s";
                debug += std::to_string(nextState->GetId());
            }
            else if (pops.size())
            {
                debug += "->pop";
            }
            else if (nextState->getIdLexAccepted()>=0)
            {
                debug += "->accept ";
                debug += std::to_string(nextState->getIdLexAccepted());
            }
            else
            {
                assert(false);
            }
            debug += ";";
        }
        if (getIdLexAccepted() >= 0)
        {
            if (pops.find(getIdLexAccepted()) == pops.end())
            {
                if (!isMainContext)
                {
                    debug += "else->s0";
                }
                else
                {
                    debug += "else->accept ";
                    debug += std::to_string(getIdLexAccepted());
                }
            }
            else
                debug += "else->pop";
            debug += ";";
        }
#ifdef TRACE_EXPREG
        debug += "    ";
        debug += m_positions.toDebugIdList();
#endif
        return debug;
    }

    void State::printAction(bool isMainContext, std::ostringstream& scode, const State* nextState, const std::set<int>& pops, const std::map<int, LexerContext*>& pushCtx, const std::map<int, std::string>& id2OnMatchCode, bool withOnFailure) const
    {
        auto iterCtx = pushCtx.find(nextState->getIdLexAccepted());
        auto mustPush = iterCtx != pushCtx.end();
        auto mustReturn = (pops.find(nextState->getIdLexAccepted()) != pops.end());
        auto iterOnMatch = id2OnMatchCode.find(nextState->getIdLexAccepted());
        std::string onMatch = (iterOnMatch != id2OnMatchCode.end()) ? iterOnMatch->second : "";
        if (mustPush)
        {
            if (mustReturn)
            {
                scode << "return " << iterCtx->second->getName() << "();";
            }
            else if (isMainContext)
            {
                scode << "if (" << iterCtx->second->getName() << "()){ accept((Lexeme)" << nextState->getIdLexAccepted() << "); return true;}else";
                if (withOnFailure)
                    scode << "{if (accepted != Lexeme::unknown_) return true; goto fail;}";
                else
                    scode << " return (accepted != Lexeme::unknown_);";
            }
            else
            {
                scode << "if (" << iterCtx->second->getName() << "()) goto state0; else " << (withOnFailure ? " goto fail;" : " return false;");
            }
        }
        else if (isMainContext)
        {
            scode << "accept((Lexeme)" << nextState->getIdLexAccepted() << ");" << " return true;";
        }
        else if (mustReturn)
        {
            if (onMatch.empty())
                scode << "return true;";
            else
                scode << "{" << onMatch << "return true;}";
        }
        else
        {
            if (onMatch.empty())
                scode << "goto " << ((GetId() == 0) ? "state0;" : "state0_noread;");
            else
                scode << "{" << onMatch << "goto " << ((GetId() == 0) ? "state0" : "state0_noread") << ";}";
        }
    }

    bool State::IsRecursiveState() const
    {
        for (const auto& transition : getTransitions())
        {
            const auto* nextState = transition.second;
            if (nextState == this)
                return true;
        }
        return false;
    }

    bool State::HasNextStateAsAcceptState() const
    {
        for (const auto& transition : getTransitions())
        {
            const auto* nextState = transition.second;
            if (nextState->getIdLexAccepted() >= 0)
                return true;
        }
        return false;
    }

    std::string State::toCpp(const std::set<int>& pops, const std::map<int, LexerContext*>& pushCtx, const std::map<int, std::string>& id2OnMatchCode, bool withNoread, bool withOnFailure) const
    {
        std::ostringstream scode;
        int indent = 2;
        int singleCharCount = 0;
        int otherSetCount = 0;
        auto isMainContext = (pops.size() == 0);

        if (isReferenced())
        {
            INDENT; scode << "state" << GetId() << ":" << std::endl;
        }
        indent++;
        if ((getIdLexAccepted() >= 0) && isMainContext)
        {
            INDENT; scode << "accept((Lexeme)" << getIdLexAccepted() << ");" << std::endl;
        }
        if ((IsRecursiveState()||(!isMainContext && HasNextStateAsAcceptState())) && (GetId() == 0))
        {
            INDENT; scode << "int c;" << std::endl;
            indent--;
            INDENT; scode << "state0:" << std::endl;
            indent++;
            INDENT;
        }
        else
        {
            INDENT; if (GetId() == 0) scode << "int ";
        }
        if (isMainContext)
        {
            scode << "c = get(); if (c==Traits::eof()) goto eof;" << std::endl;
        }
        else
        {
            scode << "c = get(); if (c==Traits::eof())" << (withOnFailure?" goto fail;":" return false;") << std::endl;
        }
        if ((GetId() == 0) && !isMainContext && withNoread)
        {
            indent--;
            INDENT; scode << "state0_noread:" << std::endl; indent++;
        }
        for (const auto& transition : getTransitions())
        {
            const auto& expr = transition.first;
            const auto* nextState = transition.second;
            if (expr.hasOnlySingleChar())
            {
                if (singleCharCount == 0)
                {
                    INDENT;
                    scode << "switch (c){" << std::endl;
                }
                auto set = expr.intersectAll();
                auto inter = std::dynamic_pointer_cast<Interval>(set);
                const Interval *interval = nullptr;
                if (!inter)
                {
                    auto multiInterval = std::dynamic_pointer_cast<MultiInterval>(set);
                    interval = &*multiInterval->getIntervals().begin();
                    if (!interval)
                        assert(false);
                }
                else
                {
                    interval = inter.get();
                }
                INDENT;
                scode << "case " << char2stringExpr(interval->getMin()) << ": ";
                if (nextState->getTransitions().size())
                    scode << "goto state" << nextState->GetId() << ";" << std::endl;
                else if (nextState->getIdLexAccepted() >= 0)
                {
                    printAction(isMainContext, scode, nextState, pops, pushCtx, id2OnMatchCode, withOnFailure);
                    scode << std::endl;
                }
                else
                    assert(false);
                singleCharCount++;
            }
            else
            {
                if (otherSetCount == 0 && singleCharCount)
                {
                    INDENT; scode << "default:" << std::endl;
                }
                INDENT;
                if (expr.size())
                {
                    // then it's an "else"
                    scode << "if (";
                    int cpt = 0;
                    bool withParenthesis = expr.size() > 1;
                    for (auto *val : expr)
                    {
                        if (cpt)
                            scode << "||";
                        if (withParenthesis)
                            scode << "(";
                        auto condition = val->toCpp("c", false);
                        if (condition.length() == 0)
                            assert(false);
                        scode << condition;
                        if (withParenthesis)
                            scode << ")";
                        cpt++;
                    }
                    scode << ") ";
                }
                if (nextState->getTransitions().size())
                {
                    scode << "goto state" << nextState->GetId() << ";" << std::endl;
                }
                else if (nextState->getIdLexAccepted() >= 0)
                {
                    printAction(isMainContext, scode, nextState, pops, pushCtx, id2OnMatchCode, withOnFailure);
                    scode << std::endl;
                }
                else
                {
                    assert(false);
                }
                otherSetCount++;
            }
        }
        if (singleCharCount)
        {
            INDENT; scode << "}" << std::endl;    // closes the switch
        }
        if (getIdLexAccepted() >= 0)
        {
            auto mustReturn = (pops.find(getIdLexAccepted()) != pops.end())||isMainContext;
            auto iterOnMatch = id2OnMatchCode.find(getIdLexAccepted());
            std::string onMatch = (!isMainContext && (iterOnMatch != id2OnMatchCode.end())) ? iterOnMatch->second : "";
            if (mustReturn)
            {
                if (onMatch.empty())
                {
                    INDENT; scode << "return true;" << std::endl;
                }
                else
                {
                    INDENT; scode << "{" << onMatch << "return true;}" << std::endl;
                }
            }
            else
            {
                if (onMatch.empty())
                {
                    INDENT; scode << "goto " << ((GetId() == 0) ? "state0;" : "state0_noread;") << std::endl;
                }
                else
                {
                    INDENT; scode << "{" << onMatch <<  "goto " << ((GetId() == 0) ? "state0;" : "state0_noread;") << "}" << std::endl;
                }
            }
        }
        else if (!isMainContext)
        {
            INDENT; scode << (withOnFailure ? "goto fail;" : "return false;") << std::endl;
        }
        else
        {
            if (withOnFailure)
            {
                INDENT; scode << "if (accepted!=Lexeme::unknown_) return true; goto fail;" << std::endl;
            }
            else
            {
                INDENT; scode << "return (accepted!=Lexeme::unknown_);" << std::endl;
            }
        }
        return scode.str();
    }

    //////////////////////////////////////////////////////////////////////////

    LexerContext::LexerContext(const std::string& name, int lexemeCount, const std::set<int>& pops)
        :m_pops(pops)
    {
        m_phead = nullptr;
        m_name = name;
        m_lexemeCount = lexemeCount;
    }

    LexerContext::LexerContext(LexerContext&& ctx) noexcept
    {
        m_phead = std::move(ctx.m_phead);
        m_mapState = std::move(ctx.m_mapState);
        m_states = std::move(ctx.m_states);
        m_AcceptState = std::move(ctx.m_AcceptState);
        m_name = std::move(ctx.m_name);
        m_lexemeCount = ctx.m_lexemeCount; ctx.m_lexemeCount = 0;
        m_pops = std::move(ctx.m_pops);
    }

    void LexerContext::setExpRegList(std::vector<std::shared_ptr<BaseLexerTreeNode>>&& list)
    {
        m_phead = std::make_unique<OrNode>(std::move(list));
    }

    void LexerContext::ComputeLexer()
    {
        std::vector<State*>   listState;
        std::map<int, short>  pos2acceptLex;

        if (!m_phead)
            return;			// !!! ERROR !!!

#ifdef TRACE_EXPREG
        std::ofstream   traceFile;
        traceFile.open("c:\\temp\\exp_reg_tree.txt", std::ios::app);
        m_phead->trace(traceFile, 0);
        traceFile.close();
#endif

        m_phead->computeFirstPos();
        m_phead->computeLastPos();
        const auto &FirstPos = m_phead->getFirstPos();
        const auto &LastPos = m_phead->getLastPos();
        m_phead->computeNextPos();

        auto iterState = m_mapState.emplace(FirstPos, State(FirstPos,0,-1));
        assert(iterState.second);   // check that starting state was added
        auto &initialState = iterState.first->second;
        listState.push_back(&initialState);
        do
        {
            std::vector<State*>   newStates;
            for (auto *state : listState)
            {
                state->ComputeNextStates(m_mapState, newStates);
                m_states.push_back(state);
            }
            listState = std::move(newStates);
        } while (listState.size());

        // simplify when possible
        bool changed = false;
        do
        {
            changed = false;

            // remove redundant transitions
            for (auto *state : m_states)
                changed |= state->removeRedundantTransitions();

            // look for duplicate states
            std::map<State*, State*> statesToBeRemoved;
            for (auto iter1 = m_states.begin();iter1!=m_states.end();)
            {
                if (iter1 != m_states.end() && statesToBeRemoved.find(*iter1) != statesToBeRemoved.end())
                    iter1++;
                if (iter1 == m_states.end())
                    break;
                auto *state1 = *iter1;
                iter1++;
                for (auto iter2 = iter1; iter2 != m_states.end();iter2++)
                {
                    auto *state2 = *iter2;
                    if (state1 != state2 && *state1 == *state2)
                        statesToBeRemoved.emplace(state2, state1);
                }
            }

            // replace everywhere reference to state2 by a reference to state1
            for (auto& pair : statesToBeRemoved)
            {
                State *oldState = pair.first;
                State *newState = pair.second;
                for (auto iter = m_states.begin(); iter != m_states.end();)
                {
                    State *state = *iter;
                    if (state == oldState)
                    {
                        iter = m_states.erase(iter);
                    }
                    else
                    {
                        changed = state->replace(oldState, newState);
                        iter++;
                    }
                }
            }
        } while (changed);

        // count the references on each state
        for (auto *state : m_states)
        {
            for (const auto &pair : state->getTransitions())
            {
                auto *s = pair.second;
                s->incrRefCount();
            }
        }
    }

    BaseLexerTreeNode *LexerContext::GetExprTree()
    {
        return m_phead.get();
    }

    const std::map<PositionSet, State>& LexerContext::getMapState() const
    {
        return m_mapState;
    }

    std::string LexerContext::declareToCpp() const
    {
        std::ostringstream scode;
        int indent = 1;

        INDENT;
        scode << "void " << m_name << "();" << std::endl;
        return scode.str();
    }

    std::string LexerContext::bodyToCpp(const std::string& onFailure, const std::map<int, std::string>& id2OnMatchCode) const
    {
        std::ostringstream scode;
        int indent = 2;
        int cpt = 0;
        auto isMainContext = (m_pops.size() == 0);

        INDENT; scode << "bool " << m_name << "(){" << std::endl; indent++;
        for (auto* state : getStates())
        {
            if (state->getTransitions().size())
            {
                scode << state->toCpp(m_pops, m_mapPush, id2OnMatchCode, m_lexemeCount>m_pops.size(), onFailure.size()>0);
                cpt++;
            }
        }
        indent--;
        if (isMainContext)
        {
            INDENT; scode << "eof:" << std::endl;
            indent++;
            INDENT; scode << "if (nbRead==0){" << std::endl;
            indent++;
            INDENT; scode << "accept(Lexeme::eof_);" << std::endl;
            INDENT; scode << "return true;" << std::endl;
            indent--;
            INDENT; scode << "}" << std::endl;
            if (onFailure.size())
            {
                INDENT; scode << "if (accepted!=Lexeme::unknown_)" << std::endl;
                indent++;
                INDENT; scode << "return true;" << std::endl;
                indent--;
            }
            else
            {
                INDENT; scode << "return (accepted!=Lexeme::unknown_);" << std::endl;
            }
            indent--;
        }
        if (onFailure.size())
        {
            INDENT; scode << "fail:" << std::endl;
            indent++;
            INDENT; scode << onFailure << std::endl;
            INDENT; scode << "return false;" << std::endl;
            indent--;
        }
        INDENT; scode << "}" << std::endl << std::endl;
        return scode.str();
    }

    std::string LexerContext::getOnMatchCode()
    {
        return "";
    }

    std::string LexerContext::getDebugString(const std::set<int>& popIds) const
    {
        std::string debug;
        for (auto state : getStates())
        {
            if (state->getTransitions().size())
                debug += state->toDebugString(popIds) + "\n";
        }
        return debug;
    }
}
