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
*  Material node
*/

#include "repo_node_material.h"

using namespace repo::core::model;

MaterialNode::MaterialNode() :
RepoNode()
{
}

MaterialNode::MaterialNode(RepoBSON bson) :
RepoNode(bson)
{

}

MaterialNode::~MaterialNode()
{
}


repo_material_t MaterialNode::getMaterialStruct() const
{
	repo_material_t mat;


	std::vector<float> tempVec = getFloatArray(REPO_NODE_MATERIAL_LABEL_AMBIENT);
	if (tempVec.size() > 0)
		mat.ambient.insert(mat.ambient.end(), tempVec.begin(), tempVec.end());

	tempVec = getFloatArray(REPO_NODE_MATERIAL_LABEL_DIFFUSE);
	if (tempVec.size() > 0)
		mat.diffuse.insert(mat.diffuse.end(), tempVec.begin(), tempVec.end());

	tempVec = getFloatArray(REPO_NODE_MATERIAL_LABEL_SPECULAR);
	if (tempVec.size() > 0)
		mat.specular.insert(mat.specular.end(), tempVec.begin(), tempVec.end());

	tempVec = getFloatArray(REPO_NODE_MATERIAL_LABEL_EMISSIVE);
	if (tempVec.size() > 0)
		mat.emissive.insert(mat.emissive.end(), tempVec.begin(), tempVec.end());

	mat.isWireframe = hasField(REPO_NODE_MATERIAL_LABEL_WIREFRAME) && 
		getField(REPO_NODE_MATERIAL_LABEL_WIREFRAME).boolean();

	mat.isTwoSided = hasField(REPO_NODE_MATERIAL_LABEL_TWO_SIDED) &&
		getField(REPO_NODE_MATERIAL_LABEL_TWO_SIDED).boolean();

	mat.opacity = hasField(REPO_NODE_MATERIAL_LABEL_OPACITY) ?
		getField(REPO_NODE_MATERIAL_LABEL_OPACITY).numberDouble() :
		std::numeric_limits<float>::quiet_NaN();

	mat.shininess = hasField(REPO_NODE_MATERIAL_LABEL_SHININESS) ?
		getField(REPO_NODE_MATERIAL_LABEL_SHININESS).numberDouble() :
		std::numeric_limits<float>::quiet_NaN();

	mat.shininessStrength = hasField(REPO_NODE_MATERIAL_LABEL_SHININESS_STRENGTH) ?
		getField(REPO_NODE_MATERIAL_LABEL_SHININESS_STRENGTH).numberDouble() :
		std::numeric_limits<float>::quiet_NaN();

	return mat;
}