/**
*  Copyright (C) 2015 3D Repo Ltd
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Affero General Public License as
*  published by the Free Software Foundation, either version 3 of the
*  License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Affero General Public License for more details.
*
*  You should have received a copy of the GNU Affero General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
* Transformation Node
*/

#pragma once
#include "repo_node.h"

namespace repo {
	namespace core {
		namespace model {
			namespace bson {

				//------------------------------------------------------------------------------
				//
				// Fields specific to transformation only
				//
				//------------------------------------------------------------------------------

				#define REPO_NODE_LABEL_MATRIX						"matrix"
				#define REPO_NODE_UUID_SUFFIX_TRANSFORMATION		"12" //!< uuid suffix
				//------------------------------------------------------------------------------

				class TransformationNode :public RepoNode
				{
					public:

						/**
						* Default constructor
						*/
						TransformationNode();

						/**
						* Construct a TransformationNode from a RepoBSON object
						* @param RepoBSON object 
						*/
						TransformationNode(RepoBSON bson);


						/**
						* Default deconstructor
						*/
						~TransformationNode();

						/**
						* Static builder for factory use to create a Transformation Node
						* @param transMatrix a 4 by 4 transformation matrix
						* @param name name of the transformation (optional)
						* @param parents parents vector of parents uuid for this node (optional)
						* @param apiLevel API Level (optional)
						* @return returns a pointer Transformation node
						*/
						static TransformationNode* createTransformationNode(
							const std::vector<std::vector<float>> &transMatrix,
							const std::string                     &name = std::string(), 
							const std::vector<repoUUID>		  &parents = std::vector<repoUUID>(),
							const int                             &apiLevel = REPO_NODE_API_LEVEL_1);

				};
			}//namespace bson
		} //namespace model
	} //namespace core
} //namespace repo

