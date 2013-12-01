/******************************************************************************
 * rTable.c                                                                    *
 *                                                                             *
 * Description: This file contains the logic operations of routing table       *
 *              actions.                                                       *
 *                                                                             *
 * Author: Fei Xie <fxie@andrew.cmu.edu>                                       *
 *                                                                             *
 *******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "rTable.h"
#include "ospf.h"

void initialize()
{
    int i, j, localFileCount, neighbourCount, sourceN;
    file_entry *tempFiles;
    neightbor_entry *tempNeighbour;

    NodeAmount = 0;
    localFileCount = 0;
    neighbourCount = 0;
    sourceN = -1;

    for (i = 0; i < MAX_NODE; i++)
    {
        for (j = 0; j < MAX_NODE; j++)
        {
            if (i == j)
            {
                matrix[i][j] = 0;
                continue;
            }
            matrix[i][j] = 9999999;
        }
    }

    /* add self to matrix */
    NodeList[NodeAmount] = nodeID;
    NodeAmount++;

    sourceN = getNodeinMatrix(nodeID);

    tempFiles = files[localFileCount];
    while (tempFiles != NULL )
    {
        localFileCount++;
        tempFiles = files[localFileCount];
    }
    objectCount[sourceN] = localFileCount;
    for (i = 0; i < localFileCount; i++)
    {
        strcpy(objectList[sourceN][i], files[i]->object);
    }

    /* add neighbour to matrix */
    tempNeighbour = neighbors[neighbourCount];
    while (tempNeighbour != NULL )
    {
        neighbourCount++;
        tempNeighbour = neighbors[neighbourCount];
    }
    neighbourCount--;

    for (i = 0; i < neighbourCount + 1; i++)
    {
        if (neighbors[i]->nodeID == nodeID)
            continue;
        NodeList[NodeAmount] = neighbors[i]->nodeID;
        NodeAmount++;
    }

    for (i = 0; i < neighbourCount; i++)
        matrix[sourceN][i + 1] = 1;
}

int32_t getNodeinMatrix(int32_t Nodeid)
{
    int32_t i;
    for (i = 0; i < NodeAmount; i++)
        if (NodeList[i] == Nodeid)
            return i;
    return -1;
}

void removeNode(int32_t Nodeid)
{
    int i, j, tempNode, plusi, plusj;
    double tempMatrix[MAX_NODE][MAX_NODE];

    tempNode = 0;
    plusi = 0;
    plusj = 0;

    if (NodeAmount < 1)
        return;

    /* delete node in node list */
    tempNode = getNodeinMatrix(Nodeid);
    for (i = tempNode; i < NodeAmount - 1; i++)
    {
        NodeList[i] = NodeList[i + 1];
    }
    NodeList[NodeAmount - 1] = -1;
    /* delete in matrix */
    for (i = 0; i < MAX_NODE; i++)
        for (j = 0; j < MAX_NODE; j++)
            tempMatrix[i][j] = matrix[i][j];

    for (i = 0; i < MAX_NODE; i++)
    {
        if (i == tempNode)
            plusi = 1;
        plusj = 0;
        for (j = 0; j < MAX_NODE; j++)
        {
            if (j == tempNode)
                plusj = 1;
            matrix[i][j] = tempMatrix[i + plusi][j + plusj];
        }
    }
    /* delete related object list */
    for (i = tempNode; i < NodeAmount - 1; i++)
        for (j = 0; j < MAX_OBJECTNUM; j++)
            memcpy(objectList[i][j], objectList[i + 1][j], 128);

    for (i = tempNode; i < NodeAmount - 1; i++)
        objectCount[i] = objectCount[i + 1];

    NodeAmount--;
    compute();
}

void updateRTable(packet_T newPacket)
{
    int32_t sourceN, sourceNode, i;
    int isNew, pos;

    sourceNode = newPacket.SenderNodeID;
    isNew = getNodeinMatrix(sourceNode);
    pos = 0;
    if (isNew == -1)
    {
        NodeList[NodeAmount] = sourceNode;
        NodeAmount++;
    }
    for (i = 0; i < newPacket.numLinkEntries; i++)
    {
        if (getNodeinMatrix(atoi(newPacket.entries + i * 5)) == -1)
        {
            NodeList[NodeAmount] = atoi(newPacket.entries + i * 5);
            NodeAmount++;
        }
    }
    /* update the matrix through the num link entry,
     * treat old and new ones the same way */
    sourceN = getNodeinMatrix(sourceNode);
    for (i = 0; i < newPacket.numLinkEntries; i++)
    {
        matrix[sourceN][getNodeinMatrix(atoi(newPacket.entries + i * 5))] = 1;
    }

    /* node id has an according object list */
    /* check if its necessary to update the object list */
    objectCount[sourceN] = newPacket.numObjectEntries; //numLinkEntries * 5 +
    for (i = 0; i < newPacket.numObjectEntries; i++)
    {
        char *buf = newPacket.entries + newPacket.numLinkEntries * 5 + pos;
        pos += strlen(buf) + 1;
        memcpy(objectList[sourceN][i], buf, strlen(buf));
    }
    compute();
}

void compute()
{
    int32_t source, i, j, Qamount, Samount, flagi, u;
    int32_t *connectList;
    int32_t *sList;
    double smallestDistance, alt;

    Qamount = NodeAmount;
    Samount = 0;
    source = getNodeinMatrix(nodeID);
    connectList = malloc(sizeof(double) * NodeAmount);
    sList = malloc(sizeof(double) * NodeAmount);
    memset(sList, 0, sizeof(double) * NodeAmount);
    smallestDistance = 999999;
    flagi = -1;
    u = -1;

    for (i = 0; i < NodeAmount; i++)
        previous[i] = -1;

    for (i = 0; i < NodeAmount; i++)
        dist[i] = 9999;
    for (i = 0; i < NodeAmount; i++)
        connectList[i] = i;
    dist[source] = 0;
    sList[0] = source;
    Samount = 1;
    while (Qamount > 0)
    {
        smallestDistance = 999999;
        flagi = -1;
        for (i = 0; i < NodeAmount; i++)
        {
            if (connectList[i] == -1)
                continue;
            for (j = 0; j < Samount; j++)
            {
                if (matrix[sList[j]][connectList[i]] < smallestDistance)
                {
                    smallestDistance = matrix[sList[j]][connectList[i]];
                    flagi = connectList[i];
                }
            }

        }
        if (flagi == -1)
        {
            Qamount--;
            continue;
        }
        u = connectList[flagi];

        connectList[flagi] = -1;
        Qamount--;
        if (u != source)
        {
            Samount++;
            sList[Samount] = u;
        }

        for (i = 0; i < NodeAmount; i++)
        {
            alt = dist[u] + matrix[u][i];
            if (alt < dist[i])
            {

                dist[i] = alt;
                if ((previous[i] == -1 || previous[i] == 0) && u != source)
                    previous[i] = u;
                else if (previous[i] == -1 && u == 0)
                    previous[i] = 0;
            }
        }
    }
    free(connectList);
    free(sList);
}

int32_t getNextHopwithObject(char *objectName)
{
    //check the previous list
    int i, j, flagi, counter, prefixLength, prefixHop, tempLength;
    double smallestV;
    char *prefix;

    prefixLength = 0;
    prefixHop = -1;
    double tempdist[MAX_NODE];

    smallestV = 9999;
    flagi = -1;
    counter = NodeAmount;

    for (i = 0; i < MAX_NODE; i++)
        tempdist[i] = dist[i];

    while (counter)
    {
        smallestV = 9999;
        flagi = -1;
        for (i = 0; i < NodeAmount; i++)
        {
            if (tempdist[i] < smallestV)
            {
                smallestV = tempdist[i];
                flagi = i;
            }
        }
        for (i = 0; i < objectCount[flagi]; i++)
        {
            if (strncmp(objectList[flagi][i], objectName, strlen(objectName))
                    == 0)
                return getNextHop(NodeList[flagi]);
            else if ((prefix = strstr(objectName, objectList[flagi][i]))
                    != NULL ) //longest prefix match
            {
                tempLength = 0;
                if (objectName[0] == prefix[0])
                {
                    for (j = 0; j < 128; j++)
                    {
                        if (objectList[flagi][i][j] != '\0')
                        {
                            tempLength++;
                            continue;
                        }
                        else
                            break;
                    }
                    if (objectList[flagi][i][tempLength - 1] == '/')
                    {
                        if (tempLength > prefixLength)
                        {
                            prefixLength = tempLength;
                            prefixHop = getNextHop(NodeList[flagi]);
                        }
                    }

                }
            }
        }
        tempdist[flagi] = 9999;
        counter--;
    }
    if (prefixHop != -1)
        return prefixHop;
    return -1;

}

int32_t getNextHop(int32_t nodeid)
{
    /* check the previous list */
    int m_node;

    m_node = getNodeinMatrix(nodeid);
    if (previous[m_node] != -1)
    {
        if (previous[m_node] == 0)
            return nodeid;
        else
            return previous[m_node];
    }
    return -1;

}
