#ifndef Graph_H
#define Graph_H
#include "../Graph/Graph.cc"
#endif

#ifndef automata_H
#include "../parseRegEx2/nodeAutomata.cc"
#define automata_H
#endif

#ifndef random_H
#define random_H
#include "../random/random.cc"
#endif

#include <omp.h>

#include "tbb/concurrent_vector.h"
#include "tbb/concurrent_unordered_map.h"

int RandomWalk(int src, int dst, Graph *g, automata *nodeAutomata, automata *edgeAutomata, Random *rand, int max_pen = 2)
{

	// walk number -> nodes visited in order
	tbb::concurrent_vector<tbb::concurrent_vector<int> > fwd_walk, bwd_walk;
	tbb::concurrent_vector<int> temp;

	for (int walknumber = 0; walknumber < g->numWalks; ++walknumber)
	{
		fwd_walk.push_back(temp);
		bwd_walk.push_back(temp);
	}

	// node -> nodestate -> penalty
	tbb::concurrent_unordered_map<int, tbb::concurrent_unordered_map<state *, int> > fwdCntr, bwdCntr;

	// node -> nodestate -> walk numbers by which it has been reached
	tbb::concurrent_unordered_map<int, tbb::concurrent_unordered_map<state *, tbb::concurrent_vector<int> > > walkNumber_F, walkNumber_B;

	bool returnzero = false, returnone = false;

	for (int walknumber2 = 0; walknumber2 < 2 * g->numWalks; walknumber2 += 4)
	{

#pragma omp parallel for schedule(static)
		for (int walknumber = walknumber2; walknumber < walknumber2+4; ++walknumber) 
		{
			//cout<<1<<endl;
			// current walk content
			unordered_set<int> set;

			// Some helpful variables
			int prev;
			Node *prevNode;
			state *prevNodeState;

			if (walknumber%2 == 0)
			{
				//cout<<2<<endl;
				bool direction = true;

				//parameter initialization
				int node = src;
				state *nodeState = nodeAutomata->startState;
				Node *currNode = g->nodes[node];

				for (int lengthCounter = 0; lengthCounter < g->walkLength; ++lengthCounter)
				{
					
					walkNumber_F[node][nodeState].push_back(walknumber / 2);
					fwd_walk[walknumber / 2].push_back(node);
					set.insert(node);

					// If no nodes then fullpenalty on node
					int numChild = (currNode->fwd_labelled_edges[1]).size();
					if (numChild == 0)
					{
						if (node == src)
						{
							returnzero = true;
							break;
						}
						fwdCntr[node][nodeState] += max_pen;
						break;
					}
					// Check for matching with already done backward walks
					if (walkNumber_B.find(node) != walkNumber_B.end())
					{
						if (walkNumber_B[node].find(nodeState) != walkNumber_B[node].end())
						{
							tbb::concurrent_vector<int> matches = walkNumber_B[node][nodeState];
							for (int i = 0; i < matches.size(); ++i)
							{
								for (int bwd_matches = 0; bwd_matches < bwd_walk[matches[i]].size(); ++bwd_matches)
								{
									if (bwd_walk[matches[i]][bwd_matches] == node)
									{
										returnone = true;
										break;
									}
									if (set.find(bwd_walk[matches[i]][bwd_matches]) != set.end())
									{
										break;
									}
								}
								if (returnone)
									break;
							}
							if (returnone)
								break;
						}
					}

					// Saving current state in case we need to fall back on them
					prevNode = currNode;
					prev = node;
					prevNodeState = nodeState;
					//cout<<3<<endl;
					// Find appropiate node label
					// 		All possible node labels
					vector<int> allNodeLabels = nodeState->labelTransitions(direction);
					if (allNodeLabels.size() == 0)
						break;

					// Randomly choosing next node
					bool flag = false;
					//cout<<4<<endl;
					for (int k = 0; k < numChild; ++k)
					{
						
						int ind = rand->next() % numChild;
						node = prevNode->fwd_labelled_edges[1][ind];
						currNode = g->nodes[node];
						//cout<<5<<endl;
						// Simplicity condition
						if (set.find(node) != set.end())
							continue;

						for (int labelChecker = 0; labelChecker < allNodeLabels.size(); ++labelChecker)
						{
							//cout<<5.1<<endl;
							int nodeLabel = allNodeLabels[labelChecker];

							vector<state *> possNewStates = prevNodeState->goTransition(nodeLabel, direction);
							ind = rand->next() % possNewStates.size();
							nodeState = possNewStates[ind];
							//cout<<5.2<<endl;
							if (currNode->labels.find(nodeLabel) != currNode->labels.end() && fwdCntr[node][nodeState] < max_pen)
							{
								flag = true;
								//cout<<5.3<<endl;
								break;
							}
						}
						if (flag)
							break;
						//cout<<6<<endl;
					}
					if (!flag)
					{

						if (++fwdCntr[prev][prevNodeState] == max_pen && prev == src)
							returnzero = true;
					}
					else
					{
						fwdCntr[prev][prevNodeState] = -max_pen;
					}
					if (!flag) break;
				}
			}
			else
			{

				bool direction = false;

				//Parameter Initialization
				int node = dst;
				state *nodeState = nodeAutomata->finalState;
				Node *currNode = g->nodes[node];

				for (int lengthCounter = 0; lengthCounter < g->walkLength; ++lengthCounter)
				{
					walkNumber_B[node][nodeState].push_back(walknumber / 2);
					bwd_walk[walknumber / 2].push_back(node);
					set.insert(node);
					// If no nodes then fullpenalty on node
					int numChild = (currNode->bwd_labelled_edges[1]).size();
					if (numChild == 0)
					{
						if (node == dst)
						{
							returnzero = true;
							break;
						}
						bwdCntr[node][nodeState] += max_pen;
						break;
					}

					// Check for matching with already done backward walks
					if (walkNumber_F.find(node) != walkNumber_F.end())
					{
						if (walkNumber_F[node].find(nodeState) != walkNumber_F[node].end())
						{
							tbb::concurrent_vector<int> matches = walkNumber_F[node][nodeState];
							for (int i = 0; i < matches.size(); ++i)
							{
								for (int fwd_matches = 0; fwd_matches < fwd_walk[matches[i]].size(); ++fwd_matches)
								{
									if (fwd_walk[matches[i]][fwd_matches] == node)
										returnone = true;
									break;
									if (set.find(fwd_walk[matches[i]][fwd_matches]) != set.end())
										break;
								}
								if (returnone)
									break;
							}
							if (returnone)
								break;
						}
					}

					// Saving current state in case we need to fall back on them
					prevNode = currNode;
					prev = node;
					prevNodeState = nodeState;

					// Find appropiate node label
					// 		All possible node labels
					vector<int> allNodeLabels = nodeState->labelTransitions(direction);
					if (allNodeLabels.size() == 0)
						break;

					// Randomly choosing next node
					bool flag = false;
					for (int k = 0; k < numChild; ++k)
					{

						int ind = rand->next() % numChild;
						node = prevNode->bwd_labelled_edges[1][ind];
						currNode = g->nodes[node];

						// Simplicity condition
						if (set.find(node) != set.end())
							continue;

						for (int labelChecker = 0; labelChecker < allNodeLabels.size(); ++labelChecker)
						{

							int nodeLabel = allNodeLabels[labelChecker];

							vector<state *> possNewStates = prevNodeState->goTransition(nodeLabel, direction);
							ind = rand->next() % possNewStates.size();
							nodeState = possNewStates[ind];

							if (currNode->labels.find(nodeLabel) != currNode->labels.end() && bwdCntr[node][nodeState] < max_pen)
							{
								flag = true;
								break;
							}
						}
						if (flag)
							break;
					}
					if (!flag)
					{
						if (++bwdCntr[prev][prevNodeState] == max_pen && prev == dst)
							returnzero = true;
					}
					else
					{
						bwdCntr[prev][prevNodeState] = -max_pen;
					}
					if (!flag) break;
				}
			}
			
		}
		if(returnone) {
			return 1;
		}
		if (returnzero) {
			return 0;
		}
	}
	return 0;
}