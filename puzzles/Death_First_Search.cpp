#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <list>

using namespace std;

/***************************************/
/* Helper classes and forward declares */
/***************************************/

#define SECURED_LINK_COST  9999.0

//----------------------- IndexedPriorityQLow ---------------------------
//
//  Priority queue based on an index into a set of keys. The queue is
//  maintained as a 2-way heap.
//
//  The priority in this implementation is the lowest valued key
//------------------------------------------------------------------------
template<class KeyType>
class IndexedPriorityQLow
{
private:

    std::vector<KeyType>& m_vecKeys;

    std::vector<int>       m_Heap;

    std::vector<int>       m_invHeap;

    int                    m_iSize,m_iMaxSize;

    void Swap(int a, int b)
    {
        int temp = m_Heap[a]; m_Heap[a] = m_Heap[b]; m_Heap[b] = temp;

        //change the handles too
        m_invHeap[m_Heap[a]] = a; m_invHeap[m_Heap[b]] = b;
    }

    void ReorderUpwards(int nd)
    {
        //move up the heap swapping the elements until the heap is ordered
        while ((nd > 1) && (m_vecKeys[m_Heap[nd / 2]] > m_vecKeys[m_Heap[nd]]))
        {
            Swap(nd / 2, nd);

            nd /= 2;
        }
    }

    void ReorderDownwards(int nd, int HeapSize)
    {
        //move down the heap from node nd swapping the elements until
        //the heap is reordered
        while (2 * nd <= HeapSize)
        {
            int child = 2 * nd;

            //set child to smaller of nd's two children
            if ((child < HeapSize) && (m_vecKeys[m_Heap[child]] > m_vecKeys[m_Heap[child + 1]]))
            {
                ++child;
            }

            //if this nd is larger than its child, swap
            if (m_vecKeys[m_Heap[nd]] > m_vecKeys[m_Heap[child]])
            {
                Swap(child, nd);

                //move the current node down the tree
                nd = child;
            }

            else
            {
                break;
            }
        }
    }


public:

    //you must pass the constructor a reference to the std::vector the PQ
    //will be indexing into and the maximum size of the queue.
    IndexedPriorityQLow(std::vector<KeyType>& keys,  int MaxSize) :m_vecKeys(keys),
        m_iMaxSize(MaxSize),
        m_iSize(0)
    {
        m_Heap.assign(MaxSize + 1, 0);
        m_invHeap.assign(MaxSize + 1, 0);
    }

    bool empty()const { return (m_iSize == 0); }

    //to insert an item into the queue it gets added to the end of the heap
    //and then the heap is reordered from the bottom up.
    void insert(const int idx)
    {
        if (m_iSize + 1 >= m_iMaxSize) {
            cerr << "m_iSize + 1 >= m_iMaxSize " << endl;
        }
        ++m_iSize;

        m_Heap[m_iSize] = idx;

        m_invHeap[idx] = m_iSize;

        ReorderUpwards(m_iSize);
    }

    //to get the min item the first element is exchanged with the lowest
    //in the heap and then the heap is reordered from the top down. 
    int Pop()
    {
        Swap(1, m_iSize);

        ReorderDownwards(1, m_iSize - 1);

        return m_Heap[m_iSize--];
    }

    //if the value of one of the client key's changes then call this with 
    //the key's index to adjust the queue accordingly
    void ChangePriority(const int idx)
    {
        ReorderUpwards(m_invHeap[idx]);
    }
};


/**********************/
/* Helper classes end */
/**********************/



//TODO: Implement dijkstra algorithm to find the closest node to the virus that has a gateway 

enum
{
    invalid_node_index = -1
};

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

class Node {
private:
    int mIndex = invalid_node_index;
    bool mGateway;

public:
    static int id;
    Node() {
        mIndex = id++;
        mGateway = false;
    }

    Node& operator=(const Node& n) {
        this->mIndex = n.mIndex;
        this->mGateway = n.mGateway;
        return *this;
    }

    int Index() const { return mIndex; }
    void SetIndex(int NewIndex) { mIndex = NewIndex; }

    bool isGateway() { return mGateway; }
    void makeGateway() { mGateway = true; }
};

int Node::id = 0;


class Edge {
public:
    Edge(const int& origin, const  int& destination) : mFrom(origin), mTo(destination),mCost(1.0), mSecured(false){ }

    int   From()const { return mFrom; }
    void  SetFrom(int NewIndex) { mFrom = NewIndex; }

    int   To()const { return mTo; }
    void  SetTo(int NewIndex) { mTo = NewIndex; }

    double Cost()const { return mCost; }
    void  SetCost(double NewCost) { mCost = NewCost; }

    bool isSecured() { return mSecured; }
    void secureLink() {
        mSecured = true;
    }

//these two operators are required
bool operator==(const Edge& rhs)
{
    return rhs.mFrom == this->mFrom &&
        rhs.mTo == this->mTo &&
        rhs.mCost == this->mCost;
}

bool operator!=(const Edge& rhs)
{
    return !(*this == rhs);
}


private:
    int mFrom;
    int mTo;
    double mCost;

    //CodingGame specific, have we secured the node already ? 
    bool mSecured = false;

//To go from origin to destination, fixed to 1 for this case
Edge() = delete;
};

string to_string(Node n) { return to_string(n.Index()); }
string to_string(Edge e) { return to_string(e.From()) + " " + to_string(e.To()); }
//string to_string(Edge& e) { return to_string(e.From()) + " " + to_string(e.To()); }

//Needed typedefs
typedef std::vector<Node>   NodeVector;
typedef std::list<Edge>     EdgeList;
typedef std::vector<EdgeList>    EdgeListVector;


//Avega: Based on SparseGraph
class GatewayNavGraph
{

public:
    friend class ConstEdgeIterator;

private:

    //the nodes that comprise this graph
    NodeVector      m_Nodes;

    //a vector of adjacency edge lists. (each node index keys into the 
    //list of edges associated with that node)
    EdgeListVector  m_Edges;

    //is this a directed graph?
    bool            m_bDigraph;

    //the index of the next node to be added
    int             m_iNextNodeIndex;

    //returns true if an edge is not already present in the graph. Used
    //when adding edges to make sure no duplicates are created.
    bool UniqueEdge(int from, int to) const
    {
        for (EdgeList::const_iterator curEdge = m_Edges[from].begin();
            curEdge != m_Edges[from].end();
            ++curEdge)
        {
            if (curEdge->To() == to)
            {
                return false;
            }
        }

        return true;
    }
    //iterates through all the edges in the graph and removes any that point
    //to an invalidated node
    void CullInvalidEdges()
    {
        for (EdgeListVector::iterator curEdgeList = m_Edges.begin(); curEdgeList != m_Edges.end(); ++curEdgeList)
        {
            for (EdgeList::iterator curEdge = (*curEdgeList).begin(); curEdge != (*curEdgeList).end(); ++curEdge)
            {
                if (m_Nodes[curEdge->To()].Index() == invalid_node_index ||
                    m_Nodes[curEdge->From()].Index() == invalid_node_index)
                {
                    curEdge = (*curEdgeList).erase(curEdge);
                }
            }
        }
    }

public:
    //ctor
    GatewayNavGraph(bool digraph) : m_iNextNodeIndex(0), m_bDigraph(digraph) {}

    const Node GetNode(int idx) {
        cerr << "(idx < (int)m_Nodes.size()) && (idx >= 0) " << bool(idx < (int)m_Nodes.size() && (idx >= 0)) << endl;
        return m_Nodes[idx];
    }

    //const method for obtaining a reference to an edge
    const Edge GetEdge(int from, int to) const {
        if (((from > m_Nodes.size()) && (from <= 0) && m_Nodes[from].Index() == invalid_node_index) == true ){ //This would be an assert, but the system does not allow it
            cerr << "(from < m_Nodes.size()) &&  (from >= 0) && m_Nodes[from].Index() != invalid_node_index" << endl;
        }
        if (((to > m_Nodes.size()) && (to <= 0) && m_Nodes[to].Index() == invalid_node_index) == true ){ //This would be an assert, but the system does not allow it
            cerr << "(to < m_Nodes.size()) && (to >= 0) && m_Nodes[to].Index() != invalid_node_index " << endl;
        }

        for (EdgeList::const_iterator curEdge = m_Edges[from].begin();
            curEdge != m_Edges[from].end();
            ++curEdge)
        {
            if (curEdge->To() == to) return *curEdge;
        }
        cerr << "Edge does not exist" << endl;
    }

    //adds a node to the graph and returns its index
    int   AddNode(Node& node) {
        if (node.Index() < (int)m_Nodes.size())
        {
            //make sure the client is not trying to add a node with the same ID as
            //a currently active node
            if (m_Nodes[node.Index()].Index() == invalid_node_index){ //This would be an assert, but the system does not allow it
                cerr << "m_Nodes[node.Index()].Index() == invalid_node_index " << endl;
            }

            m_Nodes[node.Index()] = node;

            return m_iNextNodeIndex;
        }  else {
            //make sure the new node has been indexed correctly
            if (node.Index() != m_iNextNodeIndex)//This would be an assert, but the system does not allow it
            {
                cerr << "AddNode: node.Index() != m_iNextNodeIndex [" << endl; //<GatewayNavGraph::AddNode>:invalid index
            }

            m_Nodes.push_back(node);
            m_Edges.push_back(EdgeList());

            return m_iNextNodeIndex++;
        }
    }

    //removes a node by setting its index to invalid_node_index
    void  RemoveNode(int node) {
        cerr << "node " << node << (int)m_Nodes.size()  << "(int)m_Nodes.size()" << endl;

        //set this node's index to invalid_node_index
        m_Nodes[node].SetIndex(invalid_node_index);

        //if the graph is not directed remove all edges leading to this node and then
        //clear the edges leading from the node
        if (!m_bDigraph)
        {
            //visit each neighbour and erase any edges leading to this node
            for (EdgeList::iterator curEdge = m_Edges[node].begin();
                curEdge != m_Edges[node].end();
                ++curEdge)
            {
                for (EdgeList::iterator curE = m_Edges[curEdge->To()].begin();
                    curE != m_Edges[curEdge->To()].end();
                    ++curE)
                {
                    if (curE->To() == node)
                    {
                        curE = m_Edges[curEdge->To()].erase(curE);

                        break;
                    }
                }
            }

            //finally, clear this node's edges
            m_Edges[node].clear();
        }

        //if a digraph remove the edges the slow way
        else
        {
            CullInvalidEdges();
        }
    }

    void  AddEdge(Edge edge) {

        if (bool((edge.From() < m_iNextNodeIndex) && (edge.To() < m_iNextNodeIndex)) == false) {//This would be an assert, but the system does not allow it
            cerr << "AddEdge error: (edge.From() < m_iNextNodeIndex) && (edge.To() < m_iNextNodeIndex) is false" << endl;
        }

        //make sure both nodes are active before adding the edge
        if ((m_Nodes[edge.To()].Index() != invalid_node_index) &&
            (m_Nodes[edge.From()].Index() != invalid_node_index))
        {
            //add the edge, first making sure it is unique
            if (UniqueEdge(edge.From(), edge.To()))
            {
                cerr << "Adding edge " << to_string(edge) << " to list, cost " << edge.Cost() << endl;
                m_Edges[edge.From()].push_back(edge);
            }

            //if the graph is undirected we must add another connection in the opposite
            //direction
            if (!m_bDigraph)
            {
                //check to make sure the edge is unique before adding
                if (UniqueEdge(edge.To(), edge.From()))
                {
                    Edge NewEdge = edge;

                    NewEdge.SetTo(edge.From());
                    NewEdge.SetFrom(edge.To());

                    m_Edges[edge.To()].push_back(NewEdge);
                }
            }
        }
    }

    void  RemoveEdge(int from, int to) {
        
        cerr << "(from < (int)m_Nodes.size()) && (to < (int)m_Nodes.size()" << bool((from < (int)m_Nodes.size()) && (to < (int)m_Nodes.size())) << endl;
        EdgeList::iterator curEdge;

        if (!m_bDigraph)
        {
            for (curEdge = m_Edges[to].begin();
                curEdge != m_Edges[to].end();
                ++curEdge)
            {
                if (curEdge->To() == from) { curEdge = m_Edges[to].erase(curEdge); break; }
            }
        }

        for (curEdge = m_Edges[from].begin();
            curEdge != m_Edges[from].end();
            ++curEdge)
        {
            if (curEdge->To() == to) { curEdge = m_Edges[from].erase(curEdge); break; }
        }
    }

    //sets the cost of an edge
    void  SetEdgeCost(int from, int to, double NewCost) {

        //make sure the nodes given are valid
        if ((from > m_Nodes.size()) && (to > m_Nodes.size())) {
            cerr << "((from < m_Nodes.size()) && (to < m_Nodes.size()) ; <GatewayNavGraph::SetEdgeCost>: invalid index" << endl;
        }
        cerr << "Setting new edge cost of " << NewCost << " to edge " << from << " " << to << endl;
        //visit each neighbour and erase any edges leading to this node
        for (EdgeList::iterator curEdge = m_Edges[from].begin();
            curEdge != m_Edges[from].end();
            ++curEdge)
        {
            if (curEdge->To() == to)
            {
                cerr << "New cost set correctly" << endl;
                curEdge->SetCost(NewCost);
                break;
            }
        }
    }
    //Avega: additional implementation used for Death First Search - CodinGame
    void SecureLink(int from, int to) {
        //this->GetEdge(from,to).secureLink(); //Avega: necessary ? With the cost it would be fine
        this->SetEdgeCost(from, to, SECURED_LINK_COST);
    }

    //returns the number of active + inactive nodes present in the graph
    int   NumNodes()const { return m_Nodes.size(); }

    //returns the number of active nodes present in the graph (this method's
//performance can be improved greatly by caching the value)
    int   NumActiveNodes()const
    {
        int count = 0;

        for (unsigned int n = 0; n < m_Nodes.size(); ++n) if (m_Nodes[n].Index() != invalid_node_index) ++count;

        return count;
    }

    //returns the total number of edges present in the graph
    int  NumEdges()const
    {
        int tot = 0;

        for (EdgeListVector::const_iterator curEdge = m_Edges.begin();
            curEdge != m_Edges.end();
            ++curEdge)
        {
            tot += curEdge->size();
        }

        return tot;
    }

};

//const class used to iterate through all the edges connected to a specific node. 
class ConstEdgeIterator
{
private:

    typename EdgeList::const_iterator        curEdge;

    const GatewayNavGraph& G;

    const int                                NodeIndex;

public:

    ConstEdgeIterator(const GatewayNavGraph& graph,
        int                           node) : G(graph),
        NodeIndex(node)
    {
        /* we don't need to check for an invalid node index since if the node is
           invalid there will be no associated edges
       */

        curEdge = G.m_Edges[NodeIndex].begin();
    }

    const Edge* begin()
    {
        curEdge = G.m_Edges[NodeIndex].begin();

        return &(*curEdge);
    }

    const Edge* next()
    {

        ++curEdge;
        /* avega */
        if (end()) {
            return &(*(std::prev(curEdge)));
        }
        /* avega */
        return &(*curEdge);

    }

    //return true if we are at the end of the edge list
    bool end()
    {
        //EdgeList::const_iterator next_edge = curEdge + ;
        return (curEdge == G.m_Edges[NodeIndex].end());
    }
};



//TODO: avega Review implementation of Graph_SearchDijkstra
// Specific implementation of Pathfinder just to use Dijkstra algorithm 
class Graph_SearchDijkstra
{

private:

    const GatewayNavGraph& m_Graph;

    //this vector contains the edges that comprise the shortest path tree -
    //a directed subtree of the graph that encapsulates the best paths from 
    //every node on the SPT to the source node.
    std::vector<const Edge*>      m_ShortestPathTree;

    //this is indexed into by node index and holds the total cost of the best
    //path found so far to the given node. For example, m_CostToThisNode[5]
    //will hold the total cost of all the edges that comprise the best path
    //to node 5, found so far in the search (if node 5 is present and has 
    //been visited)
    std::vector<double>            m_CostToThisNode;

    //this is an indexed (by node) vector of 'parent' edges leading to nodes 
    //connected to the SPT but that have not been added to the SPT yet. This is
    //a little like the stack or queue used in BST and DST searches.
    std::vector<const Edge*>     m_SearchFrontier;

    int                           m_iSource;
    int                           m_iTarget;
public:
    Graph_SearchDijkstra(GatewayNavGraph graph,
        int           source,
        int           target = -1) :m_Graph(graph),
        m_ShortestPathTree(graph.NumNodes()),
        m_SearchFrontier(graph.NumNodes()),
        m_CostToThisNode(graph.NumNodes()),
        m_iSource(source),
        m_iTarget(target)
    {
        Search();
    }


    ~Graph_SearchDijkstra() {  }
    //returns the vector of edges that defines the SPT. If a target was given
    //in the constructor then this will be an SPT comprising of all the nodes
    //examined before the target was found, else it will contain all the nodes
    //in the graph.
    std::vector<const Edge*> GetSPT()const { return m_ShortestPathTree; }

    //returns a vector of node indexes that comprise the shortest path
    //from the source to the target. It calculates the path by working
    //backwards through the SPT from the target node.
    std::list<int> GetPathToTarget()const {
        std::list<int> path;

        //just return an empty path if no target or no path found
        if (m_iTarget < 0)  return path;

        int nd = m_iTarget;

        path.push_front(nd);

        while ((nd != m_iSource) && (m_ShortestPathTree[nd] != 0))
        {
            nd = m_ShortestPathTree[nd]->From();

            path.push_front(nd);
        }

        return path;
    }

    //returns the total cost to the target
    double GetCostToTarget()const { return m_CostToThisNode[m_iTarget]; }

    //returns the total cost to the given node
    double GetCostToNode(unsigned int nd)const { return m_CostToThisNode[nd]; }

    //Avega - Debug: returns the vector containing the total cost to all nodes
    std::vector<double> GetCostToNodes() { return m_CostToThisNode; }

private:
    void Search() 
    {
        //create an indexed priority queue that sorts smallest to largest
        //(front to back).Note that the maximum number of elements the iPQ
        //may contain is N. This is because no node can be represented on the 
        //queue more than once.
        IndexedPriorityQLow<double> pq(m_CostToThisNode, m_Graph.NumNodes());

        //put the source node on the queue
        pq.insert(m_iSource);

        //while the queue is not empty
        while (!pq.empty())
        {
            //get lowest cost node from the queue. Don't forget, the return value
            //is a *node index*, not the node itself. This node is the node not already
            //on the SPT that is the closest to the source node
            int NextClosestNode = pq.Pop();

            //move this edge from the frontier to the shortest path tree
            m_ShortestPathTree[NextClosestNode] = m_SearchFrontier[NextClosestNode];

            //if the target has been found exit
            if (NextClosestNode == m_iTarget) {
                cerr << "Target found, returning" << endl;
                return;
            }

            //now to relax the edges.
            ConstEdgeIterator ConstEdgeItr(m_Graph, NextClosestNode);

            //for each edge connected to the next closest node
            for (const Edge* pE = ConstEdgeItr.begin();
                !ConstEdgeItr.end();
                pE = ConstEdgeItr.next())
            {
                //the total cost to the node this edge points to is the cost to the
                //current node plus the cost of the edge connecting them.
                double NewCost = m_CostToThisNode[NextClosestNode] + pE->Cost();
                cerr << "NextClosestNode is " << NextClosestNode << ", new cost to this node is " << NewCost << " = m_CostToThisNode[NextClosestNode](" << m_CostToThisNode[NextClosestNode] << ") + pE->Cost(" << (pE->Cost()) << ")" << endl;
                //if this edge has never been on the frontier make a note of the cost
                //to get to the node it points to, then add the edge to the frontier
                //and the destination node to the PQ.
                if (m_SearchFrontier[pE->To()] == 0)
                {
                    cerr << "Edge has not been in the search frontier " << endl;
                    m_CostToThisNode[pE->To()] = NewCost;

                    pq.insert(pE->To());

                    m_SearchFrontier[pE->To()] = pE;
                }

                //else test to see if the cost to reach the destination node via the
                //current node is cheaper than the cheapest cost found so far. If
                //this path is cheaper, we assign the new cost to the destination
                //node, update its entry in the PQ to reflect the change and add the
                //edge to the frontier
                else if ((NewCost < m_CostToThisNode[pE->To()]) &&
                    (m_ShortestPathTree[pE->To()] == 0))
                {
                    cerr << "Edge has been previously search frontier and the cost to this new path is smaller " << endl;

                    m_CostToThisNode[pE->To()] = NewCost;

                    //because the cost is less than it was previously, the PQ must be
                    //re-sorted to account for this.
                    pq.ChangePriority(pE->To());

                    m_SearchFrontier[pE->To()] = pE;
                }
            }
            cerr << "Cost to node " << NextClosestNode << " has been updated to " << m_CostToThisNode[NextClosestNode] " << endl;
        }


    }
    //TODO:SetSourceCell and destination possible to avoid creating more objects if we want several paths calculated ? 
};

//Avega: Dijkstra specification of the pathfinder, algorithm type is specific and brush_type has been ommited (so all costs are 1)
class Pathfinder
{
private:

    //the terrain type of each cell
    std::vector<int>              m_TerrainType;
    //this vector will store any path returned from a graph search
    std::list<int>                m_Path;
    //create a typedef for the graph type - avega: looks like not needed after the adaptation
    typedef GatewayNavGraph NavGraph;
    NavGraph* m_pGraph;
    //this vector of edges is used to store any subtree returned from 
    //any of the graph algorithms (such as an SPT)
    std::vector<const Edge*> m_SubTree;
    //the total cost of the path from target to source
    double                         m_dCostToTarget;

    //Avega: copy of the vector which contains the cost to all nodes
    std::vector<double>            m_NodesCost;

    //TODO: the current terrain brush
    //brush_type                    m_CurrentTerrainBrush;
    //the dimensions of the cells
    //double                        m_dCellWidth;
    //double                        m_dCellHeight;
    //number of cells vertically and horizontally
    int                           m_iCellsX,
        m_iCellsY;
    //local record of the client area
   // int                           m_icxClient,
     //                             m_icyClient;
    //the indices of the source and target cells
    int                           m_iSourceCell,
                                  m_iTargetCell;
    //flags to indicate if the start and finish points have been added
    bool                          m_bStart, m_bFinish; //avega: has to do with timer ? 
    //should the graph (nodes and Edges) be rendered?
    //bool                          m_bShowGraph;
    //should the tile outlines be rendered
    //bool                          m_bShowTiles;
    //this calls the appropriate algorithm
    //void  UpdateAlgorithm();


public:

    Pathfinder() :m_bStart(false),
        m_bFinish(false),
        m_iSourceCell(0),
        m_iTargetCell(0),
        //m_icxClient(0),
        //m_icyClient(0),
        m_dCostToTarget(999.0),
        m_pGraph(NULL)
    {}

    ~Pathfinder() { delete m_pGraph; }
    void InitializeGraph() {

        //delete any old graph
        delete m_pGraph;

        //create the graph
        m_pGraph = new NavGraph(true);// Avega: This problem is a directed graph https://algs4.cs.princeton.edu/42digraph/
        ResetCalculatedPathVariables();
    }

    void ResetCalculatedPathVariables() {
        m_Path.clear();
        m_SubTree.clear();
    }

    //TODO: Adapt to environment where we are creating the graph
    void CreateGraph(NodeVector &nodes, EdgeList &edges) {

        InitializeGraph();

        //first create all the nodes
        for (int n = 0; n < nodes.size(); ++n)
        {
            m_pGraph->AddNode(nodes[n]); //Also adds a empty edgeList to each node 
        }

        for (auto it = edges.begin(); it != edges.end(); it++)
        {
            //have a look at GraphHelper_AddAllNeighboursToGridNode for reference
            m_pGraph->AddEdge(*it); 
        }

    }

    //the algorithms - avega: Just Dijkstra included in this case
    void CreatePathDijkstra() {
        //set current algorithm
        //m_CurrentAlgorithm = search_dijkstra;
        //create and start a timer
        //PrecisionTimer timer; timer.Start();

        Graph_SearchDijkstra djk(*m_pGraph, m_iSourceCell, m_iTargetCell);
        
        //record the time taken  
        //m_dTimeTaken = timer.TimeElapsed();

        m_Path = djk.GetPathToTarget();

        m_SubTree = djk.GetSPT();

        m_dCostToTarget = djk.GetCostToTarget();

        m_NodesCost = djk.GetCostToNodes();
    }

    void SetSourceCell(const int cell) { m_iSourceCell = cell; }
    void SetTargetCell(const int cell) { m_iTargetCell = cell; }

    double GetCostToTarget() { return m_dCostToTarget; }
    double GetCostToNode(int node) { return m_NodesCost[node]; }


    std::list<int> getPathToTarget() { return m_Path; }

    //returns the vector of edges that defines the SPT. If a target was given
    //in the constructor then this will be an SPT comprising of all the nodes
    //examined before the target was found, else it will contain all the nodes
    //in the graph.
    std::vector<const Edge*> getSPT() { return m_SubTree; }

    //returns the terrain cost of the brush type
    //TODO: Modify for more types of terrain
    //double GetTerrainCost() { return 1.0; }

    //Avega: additional implementation used for Death First Search - CodinGame
    void secureEdgeLink(int from, int to) {
        //Edge e = m_pGraph->GetEdge(from, to);
        //e.secureLink();
        //m_pGraph->SetEdgeCost(from, to, SECURED_LINK_COST);
        m_pGraph->SecureLink(from, to);

    }

};

/*
void GraphHelper_CreateGrid(Graph_SearchDijkstra& graph,
    int cySize,
    int cxSize,
    int NumCellsY,
    int NumCellsX)
{
    //need some temporaries to help calculate each node center
    double CellWidth = (double)cySize / (double)NumCellsX;
    double CellHeight = (double)cxSize / (double)NumCellsY;

    double midX = CellWidth / 2;
    double midY = CellHeight / 2;


    //first create all the nodes
    for (int row = 0; row < NumCellsY; ++row)
    {
        for (int col = 0; col < NumCellsX; ++col)
        {
            graph.AddNode(NavGraphNode<>(graph.GetNextFreeNodeIndex(),
                Vector2D(midX + (col * CellWidth),
                    midY + (row * CellHeight))));

        }
    }
    //now to calculate the edges. (A position in a 2d array [x][y] is the
    //same as [y*NumCellsX + x] in a 1d array). Each cell has up to eight
    //neighbours.
    for (row = 0; row < NumCellsY; ++row)
    {
        for (int col = 0; col < NumCellsX; ++col)
        {
            GraphHelper_AddAllNeighboursToGridNode(graph, row, col, NumCellsX, NumCellsY);
        }
    }
}
*/

Pathfinder* g_Pathfinder;


int main()
{
    int n; // the total number of nodes in the level, including the gateways
    int l; // the number of links
    int e; // the number of exit gateways
    cin >> n >> l >> e; cin.ignore();
    /****/
    cerr << "total number of nodes in the level, including the gateways " << n << endl;
    cerr << "number of links " << l << endl;
    cerr << "number of exit gateways " << e << endl;

    g_Pathfinder = new Pathfinder();

    vector<Node> node_list, gateways;
    for (int i = 0; i < n; i++) {
        node_list.push_back(Node());

    }
    /****/
    EdgeList edge_list;
    for (int i = 0; i < l; i++) {
        int n1; // N1 and N2 defines a link between these nodes
        int n2;
        cin >> n1 >> n2; cin.ignore();
        cerr << "Link exists between node " << n1 << " and " << n2 << endl;
        Edge e (n1,n2);
        edge_list.push_back(e);
    }
    for (int i = 0; i < e; i++) {
        int ei; // the index of a gateway node
        cin >> ei; cin.ignore();
        cerr << "Node " << ei << " contains a gateway" << endl;
        node_list[ei].makeGateway();
        gateways.push_back(node_list[ei]);
    }
    //TODO: Review , not tested
    g_Pathfinder->CreateGraph(node_list, edge_list);

    /****/

    // game loop
    while (1) {
        int si; // The index of the node on which the Bobnet agent is positioned this turn
        cin >> si; cin.ignore();

        //Retrieve the target cell that the bobagent is closest to
        //For each of the gateways , compute distance 
        double minimum_distance_target = 10000;
        std::pair<Node, std::list<int>> closestGatewayPath;
        cerr << "Iterating over the different gateways:" << endl;

        for(auto node : gateways)
        {
            g_Pathfinder->ResetCalculatedPathVariables(); //For previous gateways
            g_Pathfinder->SetSourceCell(si);
            g_Pathfinder->SetTargetCell(node.Index());
            cerr << "Checking Path from Bobagent at node " << si << " to node " << to_string(node) << endl;
            g_Pathfinder->CreatePathDijkstra(); //Search is done when Path is created, so set source and target cells before that

            double cost_to_target = g_Pathfinder->GetCostToTarget();
            if (cost_to_target < minimum_distance_target) {
                minimum_distance_target = cost_to_target;
                closestGatewayPath = std::make_pair(node, g_Pathfinder->getPathToTarget());
            }

            cerr << "Cost to node "<< to_string(node) << " is " << cost_to_target << endl;
        }

        //Debug - check cost to all nodes
        cerr << "Checking total cost to all nodes :" << endl;
        for (int i = 0; i < node_list.size(); i++) {
            cerr << "Cost to node " << i << " is " << g_Pathfinder->GetCostToNode(i) << endl;
        }
        cerr << "----------------------------------------------" << endl;


        cerr << "Closest Gateway is Node " << to_string(closestGatewayPath.first) << endl;
        cerr << "Path to Gateway node the agent must  follow is";
        for (auto it = closestGatewayPath.second.begin(); it != closestGatewayPath.second.end(); it++) {
            cerr << " " << *it;
        }
        cerr << endl;

        //Eliminate the first node, which is the Bobnet agent node 
        std::list<int> gatewayPath = closestGatewayPath.second;
        //gatewayPath.pop_front();

        //Secure link between the closest node and the selected gateway
        Edge securedEdge(*(gatewayPath.begin()), *(std::next(gatewayPath.begin(), 1)));
        cerr << "Edge to secure is " << to_string(securedEdge) << endl;
        //Avega :Link is updated - secured . The cost of that edge will be a high value to indicate that the agent cannot go there easily
        g_Pathfinder->secureEdgeLink(securedEdge.From(),securedEdge.To());


        //TODO: 
        // Ensure cost is updated correctly and read by the algorithm
        // Ensure      


        // Example: 0 1 are the indices of the nodes you wish to sever the link between
        cout << to_string(securedEdge) << endl;
    }
}


