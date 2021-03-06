// $Id: CountingOcTree.cpp 252 2011-08-15 13:10:00Z ahornung $

/**
* OctoMap:
* A probabilistic, flexible, and compact 3D mapping library for robotic systems.
* @author K. M. Wurm, A. Hornung, University of Freiburg, Copyright (C) 2009.
* @see http://octomap.sourceforge.net/
* License: New BSD License
*/

/*
 * Copyright (c) 2009-2011, K. M. Wurm, A. Hornung, University of Freiburg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Freiburg nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cassert>
#include <octomap/CountingOcTree.h>

namespace octomap {


  /// implementation of CountingOcTreeNode  ----------------------------------

  CountingOcTreeNode::CountingOcTreeNode()
    : OcTreeDataNode<unsigned int>(0)
  {
  }

  CountingOcTreeNode::~CountingOcTreeNode() {

  }

  void CountingOcTreeNode::expandNode(){
    assert(!hasChildren());

    // divide "counts" evenly to children
    unsigned int childCount = (unsigned int)(value/ 8.0 +0.5);
    for (unsigned int k=0; k<8; k++) {
      createChild(k);
      itsChildren[k]->setValue(childCount);
    }
  }

  bool CountingOcTreeNode::createChild(unsigned int i) {
    if (itsChildren == NULL) {
      allocChildren();
    }
    assert (itsChildren[i] == NULL);
    itsChildren[i] = new CountingOcTreeNode();
    return true;
  }


  /// implementation of CountingOcTree  --------------------------------------

  CountingOcTree::CountingOcTree(double _resolution)
    : OcTreeBase<CountingOcTreeNode>(_resolution)   {

    itsRoot = new CountingOcTreeNode();
    tree_size++;
  }


  CountingOcTreeNode* CountingOcTree::updateNode(const point3d& value) {

    OcTreeKey key;
    if (!genKey(value, key)) return NULL;
    return updateNode(key);
  }


  // Note: do not inline this method, will decrease speed (KMW)
  CountingOcTreeNode* CountingOcTree::updateNode(const OcTreeKey& k) {

    CountingOcTreeNode* curNode (itsRoot);
    curNode->increaseCount();
    unsigned int pos(0);

    // follow or construct nodes down to last level...
    for (int i=(tree_depth-1); i>=0; i--) {

      genPos(k, i, pos);

      // requested node does not exist
      if (!curNode->childExists(pos)) {
        curNode->createChild(pos);
        tree_size++;
      }
      // descent tree
      curNode = static_cast<CountingOcTreeNode*> (curNode->getChild(pos));
      curNode->increaseCount(); // modify traversed nodes
    }

    return curNode;
  }


  void CountingOcTree::getCentersMinHits(point3d_list& node_centers, unsigned int min_hits) const {

    OcTreeKey root_key;
    root_key[0] = root_key[1] = root_key[2] = this->tree_max_val;
    getCentersMinHitsRecurs(node_centers, min_hits, this->tree_depth, this->itsRoot, 0, root_key);    
  }


  void CountingOcTree::getCentersMinHitsRecurs( point3d_list& node_centers,
                                                unsigned int& min_hits,
                                                unsigned int max_depth,
                                                CountingOcTreeNode* node, unsigned int depth,
                                                const OcTreeKey& parent_key) const {

    if (depth < max_depth && node->hasChildren()) {

      unsigned short int center_offset_key = this->tree_max_val >> (depth + 1);
      OcTreeKey search_key;

      for (unsigned int i=0; i<8; ++i) {
        if (node->childExists(i)) {
          computeChildKey(i, center_offset_key, parent_key, search_key);
          getCentersMinHitsRecurs(node_centers, min_hits, max_depth, node->getChild(i), depth+1, search_key);
        }
      }
    }

    else { // max level reached

      if (node->getCount() >= min_hits) {
        point3d p;
	this->genCoords(parent_key, depth, p);
        node_centers.push_back(p);        
      }
    }
  }


} // namespace
