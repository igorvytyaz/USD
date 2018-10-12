//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//

#ifndef USDDRACO_EXPORT_TRANSLATOR_H
#define USDDRACO_EXPORT_TRANSLATOR_H

#include "attributeDescriptor.h"
#include "importAttribute.h"

#include "pxr/pxr.h"
#include "pxr/usd/usdGeom/mesh.h"

#include <draco/mesh/corner_table.h>
#include <draco/mesh/mesh.h>


PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdDracoImportTranslator
///
/// Translates Draco mesh to USD mesh.
///
class UsdDracoImportTranslator {
public:
    /// Translates Draco mesh to USD mesh and returns USD layer with mesh.
    static SdfLayerRefPtr Translate(const draco::Mesh &dracoMesh);

private:
    typedef draco::AttributeValueIndex PositionIndex;
    typedef std::map<PositionIndex, draco::PointIndex> PolygonEdges;

    /// Constructor.
    UsdDracoImportTranslator(const draco::Mesh &dracoMesh);

    SdfLayerRefPtr _Translate();
    VtVec3fArray _ComputeExtent() const;
    bool _CheckData() const;
    void _PopulateValuesFromMesh();
    void _PopulateIndicesFromMesh();
    void _SetAttributesToMesh(UsdGeomMesh *usdMesh) const;
    bool _HasTrianglesOnly() const;
    bool _SubdivisionRefersToPositions(const UsdGeomMesh &usdMesh) const;
    bool _SubdivisionRefersToFaces(const UsdGeomMesh &usdMesh) const;
    void _FindOriginalFaceEdges(draco::FaceIndex faceIndex,
                                const draco::CornerTable *cornerTable,
                                std::vector<bool> &triangleVisited,
                                PolygonEdges &polygonEdges);

private:
    const draco::Mesh &_dracoMesh;

    UsdDracoImportAttribute<VtVec3fArray> _positions;
    UsdDracoImportAttribute<VtVec2fArray> _texCoords;
    UsdDracoImportAttribute<VtVec3fArray> _normals;
    UsdDracoImportAttribute<VtIntArray> _holeFaces;
    UsdDracoImportAttribute<VtIntArray> _addedEdges;
    UsdDracoImportAttribute<VtIntArray> _posOrder;

    VtIntArray _faceVertexCounts;
    VtIntArray _faceVertexIndices;
    VtIntArray _holeIndices;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif  // USDDRACO_EXPORT_TRANSLATOR_H
