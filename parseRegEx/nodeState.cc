#include "nodeState.h"

typedef tbb::mutex myMutex;
static myMutex sm;

state::state(int _label) {
    label = _label;
    fdone = false;
    bdone = false;
}

void state::setTransition(state* _next, bool _dir) {
    if (_dir) {
        fwdTransitions.push_back(_next);
    }
    else {
        bwdTransitions.push_back(_next);
    }
}

vector < state* > state::posTransition(int label, bool _dir) {
    vector <state*> out;
    if (_dir) {
        for (int i = 0; i < fwdTransitions.size(); ++i) {
            if (fwdTransitions[i]->label == label) {
                out.push_back(fwdTransitions[i]);
            } 
        }
    }
    else {
        for (int i = 0; i < bwdTransitions.size(); ++i) {
            if (bwdTransitions[i]->label == label) {
                out.push_back(bwdTransitions[i]);
            } 
        }
    }
    return out;
}

vector < int > state::labelTransitions(bool _dir) {
    if (_dir) {
        if (!fdone) {
            myMutex::scoped_lock lock;
            lock.acquire(sm);
            if (labelTransitionsF.empty()) {
                for (int i = 0; i < fwdTransitions.size(); ++i) {
                    labelTransitionsF.push_back(fwdTransitions[i]->label);
                    labelToStateF[fwdTransitions[i]->label].push_back(fwdTransitions[i]);
                }
                fdone = true;
            }
            lock.release();
        }
        return labelTransitionsF;
    }
    else {
        if (!bdone) {
            myMutex::scoped_lock lock;
            lock.acquire(sm);
            if (labelTransitionsB.empty()) {
                for (int i = 0; i < bwdTransitions.size(); ++i) {
                    labelTransitionsB.push_back(bwdTransitions.at(i)->label);
                    labelToStateB[bwdTransitions.at(i)->label].push_back(bwdTransitions.at(i));
                }
                bdone = true;
            }
            lock.release();
        }
        return labelTransitionsB;
    }
}

vector < state* > state::goTransition(int label, bool _dir) {
    if (_dir) return labelToStateF.at(label);
    else return labelToStateB.at(label);
}