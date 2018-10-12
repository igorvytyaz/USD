//
// Copyright 2018 Pixar
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

#ifndef USDDRACO_IMPORT_ATTRIBUTE_H
#define USDDRACO_IMPORT_ATTRIBUTE_H

#include "attributeDescriptor.h"

#include "pxr/pxr.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"

#include <draco/attributes/point_attribute.h>
#include <draco/mesh/mesh.h>


PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdDracoImportAttribute
///
/// Helps to read and write mesh attributes while importing Draco files to USD.
///
template <class ArrayT>
class UsdDracoImportAttribute {
public:
    UsdDracoImportAttribute(UsdDracoAttributeDescriptor descriptor,
                            const draco::Mesh &dracoMesh);
    void SetToMesh(UsdGeomMesh *usdMesh) const;
    void PopulateValues();
    void PopulateValuesWithOrder(
        const UsdDracoImportAttribute<VtIntArray> &order, size_t numFaces,
        const draco::Mesh &dracoMesh);
    int GetMappedValue(draco::PointIndex pi) const;
    int GetMappedIndex(draco::PointIndex pi) const;
    const ArrayT &GetValues() const;
    void ResizeIndices(size_t size);
    void SetIndex(size_t at, int index);
    size_t GetNumValues() const;
    size_t GetNumIndices() const;
    bool HasPointAttribute() const;

private:
    const draco::PointAttribute *_GetFromMesh(
        const draco::Mesh &dracoMesh);

private:
    const UsdDracoAttributeDescriptor _descriptor;
    const draco::PointAttribute *_pointAttribute;
    ArrayT _values;
    VtIntArray _indices;
};


template <class ArrayT>
UsdDracoImportAttribute<ArrayT>::UsdDracoImportAttribute(
        UsdDracoAttributeDescriptor descriptor, const draco::Mesh &dracoMesh) :
            _descriptor(descriptor), _pointAttribute(_GetFromMesh(dracoMesh)) {
}

template <class ArrayT>
const draco::PointAttribute *UsdDracoImportAttribute<ArrayT>::_GetFromMesh(
        const draco::Mesh &dracoMesh) {
    const int attributeId = _descriptor.metadataName.empty()
        ? dracoMesh.GetNamedAttributeId(_descriptor.attributeType)
        : dracoMesh.GetAttributeIdByMetadataEntry(
            UsdDracoAttributeDescriptor::METADATA_NAME_KEY,
            _descriptor.metadataName);
    return (attributeId == -1) ? nullptr : dracoMesh.attribute(attributeId);
}

template <class ArrayT>
void UsdDracoImportAttribute<ArrayT>::SetToMesh(UsdGeomMesh *usdMesh) const {
    if (_pointAttribute == nullptr)
        return;
    if (_descriptor.isPrimvar) {
        // Set data as a primvar.
        const UsdGeomPrimvarsAPI api = UsdGeomPrimvarsAPI(usdMesh->GetPrim());
        UsdGeomPrimvar primvar = api.CreatePrimvar(
            _descriptor.name, _descriptor.valueType);
        primvar.Set(_values);
        primvar.SetIndices(_indices);
        primvar.SetInterpolation(UsdGeomTokens->faceVarying);
    } else {
        // Set data as an attribute.
        UsdAttribute attribute = usdMesh->GetPrim().CreateAttribute(
            _descriptor.name, _descriptor.valueType);
        attribute.Set(_values);
    }
}

template <class ArrayT>
void UsdDracoImportAttribute<ArrayT>::PopulateValues() {
    if (_pointAttribute == nullptr)
        return;
    const size_t numValues = _pointAttribute->size();
    _values.resize(numValues);
    for (size_t i = 0; i < numValues; i++) {
        draco::AttributeValueIndex avi(i);
        _pointAttribute->GetValue(avi, _values[i].data());
    }
}

template <class ArrayT>
void UsdDracoImportAttribute<ArrayT>::PopulateValuesWithOrder(
        const UsdDracoImportAttribute<VtIntArray> &order, size_t numFaces,
        const draco::Mesh &dracoMesh) {
    if (_pointAttribute == nullptr)
        return;
    const size_t numValues = _pointAttribute->size();
    _values.resize(numValues);
    std::vector<bool> populated(numValues, false);
    for (size_t i = 0; i < numFaces; i++) {
        const draco::Mesh::Face &face = dracoMesh.face(draco::FaceIndex(i));
        for (size_t c = 0; c < 3; c++) {
            const draco::PointIndex pi = face[c];
            int origIndex = order.GetMappedValue(pi);
            if (!populated[origIndex]) {
                _pointAttribute->GetMappedValue(pi, _values[origIndex].data());
                populated[origIndex] = true;
            }
        }
    }
}

template <class ArrayT>
inline int UsdDracoImportAttribute<ArrayT>::GetMappedValue(
        draco::PointIndex pi) const {
    int index;
    _pointAttribute->GetMappedValue(pi, &index);
    return index;
}

template <class ArrayT>
inline int UsdDracoImportAttribute<ArrayT>::GetMappedIndex(
        draco::PointIndex pi) const {
    return static_cast<int>(_pointAttribute->mapped_index(pi).value());
}

template <class ArrayT>
const ArrayT &UsdDracoImportAttribute<ArrayT>::GetValues() const {
    return _values;
}

template <class ArrayT>
void UsdDracoImportAttribute<ArrayT>::ResizeIndices(size_t size) {
    if (_pointAttribute == nullptr)
        return;
    _indices.resize(size);
}

template <class ArrayT>
void UsdDracoImportAttribute<ArrayT>::SetIndex(size_t at, int index) {
    _indices[at] = index;
}

template <class ArrayT>
size_t UsdDracoImportAttribute<ArrayT>::GetNumValues() const {
    return _values.size();
}

template <class ArrayT>
size_t UsdDracoImportAttribute<ArrayT>::GetNumIndices() const {
    return _indices.size();
}

template <class ArrayT>
inline bool UsdDracoImportAttribute<ArrayT>::HasPointAttribute() const {
    return _pointAttribute != nullptr;
}


PXR_NAMESPACE_CLOSE_SCOPE

#endif  // USDDRACO_IMPORT_ATTRIBUTE_H
