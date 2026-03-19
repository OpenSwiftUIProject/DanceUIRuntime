// Copyright (c) 2025 ByteDance Ltd. and/or its affiliates
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <DanceUIRuntime/metadata_visitor.hpp>
#include <DanceUIRuntime/LayoutDescriptor.hpp>
#include <DanceUIRuntime/DanceUITypeApplyOptions.h>

using namespace swift;
using namespace DanceUI;

enum DanceUIApplyOptions: int64_t {
    NonHeap = 0x0,
    Heap = 0x1,
    ContinueWhenUnknown = 0x2,
    AllowVisitEnum = 0x4,
};

DANCE_UI_INLINE
DanceUIApplyOptions operator &(const DanceUIApplyOptions &lhs,
                               const DanceUIApplyOptions &rhs) {
    return DanceUIApplyOptions(int64_t(lhs) & int64_t(rhs));
}

namespace DanceUI {

namespace Swift {

struct TypeApplyFields {

public:
    class Visitor final: public metadata_visitor {
    public:
        Visitor(const void *fn, void *context) noexcept :
        metadata_visitor(), _fn(fn), _context(context) {
        }

    public:

        virtual bool unknown_result() const noexcept {
            return true;
        }

        virtual bool visit_field(const metadata *metadata,
                                 const field_record &field_record,
                                 size_t offset,
                                 size_t remaining_size) noexcept {
            auto type = metadata->mangled_type_name_ref(field_record.MangledTypeName.get(),
                                                        nullptr);
            if (!type) {
                return true;
            }
            auto fn = ((SWIFT_CC(swift) bool(*)(const char *,
                                                size_t,
                                                const Metadata *,
                                                SWIFT_CONTEXT void *))_fn);
            fn(field_record.FieldName.get(), offset, type, _context);
            return true;
        }
    private:
        const void *_fn;
        void *_context;
    };

};

struct TypeApplyFields2 {

public:
    class Visitor final: public metadata_visitor {
    public:
        Visitor(DanceUIApplyOptions options,
                const void *fn,
                void *context) :
        metadata_visitor(),
        _fn(fn),
        _context(context),
        _options(options) {
        }

    public:

        virtual bool unknown_result() noexcept {
            return (_options & DanceUIApplyOptions::ContinueWhenUnknown) ==
            DanceUIApplyOptions::ContinueWhenUnknown;
        }

        virtual bool visit_field(const metadata *metadata,
                                 const field_record &field_record,
                                 size_t offset,
                                 size_t remaining_size) noexcept {
            auto type = metadata->mangled_type_name_ref(field_record.MangledTypeName.get(),
                                                        nullptr);
            if (!type) {
                return unknown_result();
            }
            auto fn = ((SWIFT_CC(swift) bool(*)(const char *,
                                                size_t,
                                                const Metadata *,
                                                SWIFT_CONTEXT void *))_fn);
            return fn(field_record.FieldName.get(), offset, type, _context);
        }

        virtual bool visit_case(const metadata *metadata,
                                const field_record &field_record,
                                size_t case_index) noexcept {
            auto type = metadata->mangled_type_name_ref(field_record.MangledTypeName.get(),
                                                        nullptr);
            if (!type) {
                return unknown_result();
            }
            auto fn = ((SWIFT_CC(swift) bool(*)(const char *,
                                                size_t,
                                                const Metadata *,
                                                SWIFT_CONTEXT void *))_fn);
            return fn(field_record.FieldName.get(), case_index, type, _context);
        }
    private:
        const void *_fn;
        void *_context;
        const DanceUIApplyOptions _options;
    };

};

}

}

extern "C"
void DanceUIApplyFields(const Metadata *type,
                        const void *fn,
                        void *ctx) {
    const Swift::metadata *metadata = reinterpret_cast<const Swift::metadata *>(type);

    Swift::TypeApplyFields::Visitor visitor(fn, ctx);
    metadata->visit(visitor);
    return;
}

extern "C"
bool DanceUIApplyFields2(const Metadata *type,
                         DanceUITypeApplyOptions _options,
                         const void *fn,
                         void *ctx) {
    const Swift::metadata *metadata = reinterpret_cast<const Swift::metadata *>(type);
    const DanceUIApplyOptions options = DanceUIApplyOptions(_options);
    Swift::TypeApplyFields2::Visitor visitor(options,
                                             fn,
                                             ctx);
    switch (metadata->getKind()) {
        case MetadataKind::Struct:
        case MetadataKind::Tuple: {

            if (options & (DanceUIApplyOptions::Heap | DanceUIApplyOptions::AllowVisitEnum)) {
                return false;
            }
            return metadata->visit(visitor);
        }
        case MetadataKind::Enum:
        case MetadataKind::Optional: {
            if (options & (DanceUIApplyOptions::AllowVisitEnum)) {
                return metadata->visit(visitor);
            }
            return false;
        }
        case MetadataKind::Class: {

            if ((options & DanceUIApplyOptions::Heap) !=
                DanceUIApplyOptions::Heap) {
                return false;
            }
            return metadata->visit_heap(visitor,
                                       uint32_t(LayoutDescriptor::HeapType::Class));
        }
        default:
            return false;
    }
}
