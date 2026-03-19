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

#include <DanceUIRuntime/LayoutDescriptor.hpp>
#include <DanceUIRuntime/metadata_visitor.hpp>
#include <DanceUIRuntime/vector.hpp>
#include <DanceUIRuntime/DanceUIOpaqueValueProjection.hpp>
#include <DanceUIRuntime/DebugEnvironments.hpp>
#include <DanceUIRuntime/EnvironmentVariable.hpp>

#include <swift/Runtime/Enum.h>

#include <boost/mpl/index_of.hpp>
#include <iostream>
#include <iomanip>

#include <mach/mach_time.h>

extern "C"
bool DanceUICompareValues(const OpaqueValue *lhs,
                          const OpaqueValue *rhs,
                          const Metadata *type,
                          const DanceUIComparisonMode comparisonMode);

extern "C"
bool DanceUICompareValuesPartial(const OpaqueValue *lhs,
                                 const OpaqueValue *rhs,
                                 size_t offset,
                                 size_t size,
                                 const DanceUIComparisonMode comparisonMode,
                                 const Metadata *type);

static_assert(_LIBCPP_STD_VER > 14, "DanceUI/Runtime was using C++17 Mode!");

namespace DanceUI {

struct TypeDescriptorKey {

    DANCE_UI_INLINE
    TypeDescriptorKey(const std::pair<const Swift::metadata *,
                      const std::pair<const DanceUIComparisonMode,
                      const LayoutDescriptor::HeapMode>> &value) noexcept :
    first(value.first),
    second(value.second.first),
    third(value.second.second) {
    }

    DANCE_UI_INLINE
    TypeDescriptorKey(const TypeDescriptorKey &value) noexcept :
    first(value.first), second(value.second), third(value.third) {
    }

    DANCE_UI_INLINE
    TypeDescriptorKey(TypeDescriptorKey &&value) noexcept :
    first(value.first), second(value.second), third(value.third) {
    }

    const Swift::metadata *first;
    const DanceUIComparisonMode second;
    const LayoutDescriptor::HeapMode third;

    DANCE_UI_INLINE
    bool operator==(const TypeDescriptorKey &other) const noexcept {
        return (first == other.first &&
                second == other.second &&
                third == other.third);
    }
};
}

#include <boost/functional/hash.hpp>

namespace std {

template <>
struct hash<DanceUI::TypeDescriptorKey> {

    DANCE_UI_INLINE
    std::size_t operator()(const DanceUI::TypeDescriptorKey& k) const noexcept {

        using boost::hash_value;
        using boost::hash_combine;

        std::size_t seed = 0;

        hash_combine(seed, hash_value(k.first));
        hash_combine(seed, hash_value(k.second));
        hash_combine(seed, hash_value(k.third));

        return seed;
    }
};

}

namespace DanceUI {

DANCE_UI_INLINE
const OpaqueValue *projection(const OpaqueValue *value,
                              const LayoutDescriptor::Builder::RangeItem &item) {
    return projection(value, item.start_offset);
}

DANCE_UI_INLINE
bool __compare_bytes(const void *lhs, const void *rhs, size_t length) noexcept {
    return __builtin_memcmp(lhs, rhs, length) == 0;
}

bool compare_bytes(const void *lhs, const void *rhs, size_t length) noexcept {
    return __compare_bytes(lhs, rhs, length);
}

namespace {

template <typename Sequence, typename T>
constexpr size_t get_index_of() noexcept {
    return boost::mpl::index_of<Sequence, T>::type::value;
}

class TypeDescriptorCache final {
public:

#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)

    class ProfileData final {
    public:
        ProfileData() noexcept {
            const int32_t loop_count = 0x10;
            uint64_t previous_time = mach_absolute_time();
            int32_t loop_index = 0;

            do {
                const uint64_t current_time = mach_absolute_time();
                const uint64_t delta = current_time - previous_time;

                _start_time_base += delta;
                previous_time = current_time;
                loop_index ++;
            } while (loop_index < loop_count);
            _start_time_base /= loop_count;

            struct mach_timebase_info info;
            const kern_return_t ret = mach_timebase_info(&info);
            if (ret != KERN_SUCCESS) {
                return;
            }
            _frequency = (double)1.0e9 * ((double)info.denom / (double)info.numer);
        }

        ProfileData(const ProfileData &) noexcept = default;
        ProfileData(ProfileData &&) noexcept = default;

        void print(std::ostream &os) const noexcept {
            os << "query: " << get_query_count() << ";" << std::endl;
            os << "query-hit: " << get_query_hit_count() << ";" << std::endl;
            os << "query-missing: " << get_query_missing_count() << ";" << std::endl;
            os << "query-invalidate: " << get_query_invalidate_count() << ";" << std::endl;
            os << "query-hit-rate: " << std::fixed << std::setprecision(3) << double(get_query_hit_count()) / double(get_query_count()) * 100.0 << "%;" << std::endl;
            os << "query-invalidate-rate: " << std::setprecision(5) << double(get_query_invalidate_count()) / double(get_query_count()) * 100.0 << "%;" << std::endl;

            os << "============================================" << std::endl;
            os << "make-layout: " << get_make_layout_count() << ";" << std::endl;
            os << "make-layout-async: " << get_make_layout_async_count() << ";" << std::endl;
            os << "make-layout-sync: " << get_make_layout_sync_count() << ";" << std::endl;
            os << "make-layout-async-rate: " << std::fixed << std::setprecision(3) << double(get_make_layout_async_count()) / double(get_make_layout_count()) * 100.0 << "%;" << std::endl;

            os << "============================================" << std::endl;
            os << "fetch-total: " << std::fixed << std::setprecision(6) << get_fetch_total() / _frequency * 1000.0 << "ms;" << std::endl;
            os << "fetch-max: " << std::fixed << std::setprecision(6) << get_fetch_max() / _frequency * 1000.0 << "ms;" << std::endl;
            os << "fetch-min: " << std::fixed << std::setprecision(6) << get_fetch_min() / _frequency * 1000.0 << "ms;" << std::endl;

            os << "============================================" << std::endl;
        }

        DANCE_UI_INLINE
        uint64_t get_query_count(void) const noexcept {
            return get_query_hit_count() + get_query_missing_count();
        }

        DANCE_UI_INLINE
        uint64_t get_query_hit_count(void) const noexcept {
            return _query_hit_count;
        }

        DANCE_UI_INLINE
        uint64_t get_query_missing_count(void) const noexcept {
            return _query_missing_count;
        }

        DANCE_UI_INLINE
        uint64_t get_make_layout_count(void) const noexcept {
            return get_make_layout_async_count() + get_make_layout_sync_count();
        }

        DANCE_UI_INLINE
        uint64_t get_make_layout_async_count(void) const noexcept {
            return _make_layout_async_count;
        }

        DANCE_UI_INLINE
        uint64_t get_make_layout_sync_count(void) const noexcept {
            return _make_layout_sync_count;
        }

        DANCE_UI_INLINE
        uint64_t get_query_invalidate_count(void) const noexcept {
            return _query_invalidate_count;
        }

        DANCE_UI_INLINE
        uint64_t get_fetch_total(void) const noexcept {
            return _fetch_total;
        }

        DANCE_UI_INLINE
        uint64_t get_fetch_max(void) const noexcept {
            return _fetch_max;
        }

        DANCE_UI_INLINE
        uint64_t get_fetch_min(void) const noexcept {
            return _fetch_min;
        }

        void add_fetch(const Metadata *metadata,
                       uint64_t time,
                       bool asynchronous,
                       bool invalidate) noexcept {
            time -= _start_time_base;

            _data[metadata].add_fetch(time, invalidate);

            _fetch_total += time;
            _fetch_max = std::max(_fetch_max, time);
            _fetch_min = std::min(_fetch_min, time);
        }

    private:

        class FetchProfile final {
        public:

            void add_fetch(uint64_t time, bool invalidate) noexcept {
                fetch_count += 1;

                fetch_total += time;
                fetch_min = std::min(fetch_min, time);
                fetch_max = std::max(fetch_max, time);

                if (!invalidate) {
                    validate_count += 1;

                    validate_total += time;
                    validate_min = std::min(validate_min, time);
                    validate_max = std::max(validate_max, time);
                } else {
                    wasted_total += time;
                    wasted_min = std::min(wasted_min, time);
                    wasted_max = std::max(wasted_max, time);
                }
            }

            FetchProfile &operator+=(const FetchProfile &rhs) noexcept {
                fetch_count += rhs.fetch_count;
                fetch_total += rhs.fetch_total;

                fetch_min = std::min(fetch_min, rhs.fetch_min);
                fetch_max = std::max(fetch_max, rhs.fetch_max);

                validate_count += rhs.validate_count;
                validate_total += rhs.validate_total;

                validate_min = std::min(validate_min, rhs.validate_min);
                validate_max = std::max(validate_max, rhs.validate_max);

                wasted_total += rhs.wasted_total;
                wasted_min = std::min(wasted_min, rhs.wasted_min);
                wasted_max = std::max(wasted_max, rhs.wasted_max);

                return *this;
            }


            uint64_t fetch_count = 0;


            uint64_t validate_count = 0;


            uint64_t fetch_total = 0;


            uint64_t fetch_min = 0;


            uint64_t fetch_max = 0;


            uint64_t validate_total = 0;


            uint64_t validate_min = 0;


            uint64_t validate_max = 0;


            uint64_t wasted_total = 0;


            uint64_t wasted_min = 0;


            uint64_t wasted_max = 0;
        };

        friend class TypeDescriptorCache;

        DANCE_UI_INLINE
        void _inc_query_hit_count(void) noexcept {
            _query_hit_count++;
        }

        DANCE_UI_INLINE
        void _inc_query_missing_count(void) noexcept {
            _query_missing_count++;
        }

        DANCE_UI_INLINE
        void _inc_make_layout_sync_count(void) noexcept {
            _make_layout_sync_count++;
        }

        DANCE_UI_INLINE
        void _inc_make_layout_async_count(void) noexcept {
            _make_layout_async_count++;
        }

        DANCE_UI_INLINE
        void _inc_query_invalidate_count(void) noexcept {
            _query_invalidate_count++;
        }

    private:

        double _frequency = 0.0;
        uint64_t _start_time_base = 0;

        uint64_t _query_hit_count = 0;
        uint64_t _query_missing_count = 0;
        uint64_t _make_layout_sync_count = 0;
        uint64_t _make_layout_async_count = 0;

        uint64_t _query_invalidate_count = 0;

        std::unordered_map<const Metadata *, FetchProfile> _data;
        uint64_t _fetch_total = std::numeric_limits<uint64_t>::min();
        uint64_t _fetch_max = std::numeric_limits<uint64_t>::min();
        uint64_t _fetch_min = std::numeric_limits<uint64_t>::max();
    };

    static void print(std::ostream &os) noexcept {
        auto &instance = init_shared_cache();
        std::lock_guard<TypeDescriptorCache> lock_guard(instance);
        if (!instance.has_profile_data()) {
            return;
        }
        return instance.get_profile_data()->print(os);
    }

#endif

    DANCE_UI_INLINE
    static
    DanceUIComparisonMode fetch_mode(const void *type_descriptor, DanceUIComparisonMode default_comparison_mode) {
        auto &instance = init_shared_cache();
        std::lock_guard<TypeDescriptorCache> lock(instance);
        std::pair<const void *, DanceUIComparisonMode> key = {type_descriptor, default_comparison_mode};
        auto found = std::lower_bound(instance._overriden_type_descriptors.begin(), instance._overriden_type_descriptors.end(), key, [](const auto &lhs, const auto &rhs) {
            return lhs.first < rhs.first;
        });
        if (found != instance._overriden_type_descriptors.end()) {
            return found->second;
        }
        return default_comparison_mode;
    }

    DANCE_UI_INLINE
    static
    void add_type_descriptor_override(const void *type_descriptor, DanceUIComparisonMode comparison_mode) {
        if (type_descriptor == nullptr) {
            return;
        }
        auto &instance = init_shared_cache();
        std::lock_guard<TypeDescriptorCache> lock(instance);
        std::pair<const void *, DanceUIComparisonMode> key = {type_descriptor, comparison_mode};
        auto found = std::lower_bound(instance._overriden_type_descriptors.begin(), instance._overriden_type_descriptors.end(), key, [](const auto &lhs, const auto &rhs) {
            return lhs.first < rhs.first;
        });
        if (found != instance._overriden_type_descriptors.end()) {
            found->second = comparison_mode;
        } else {
            instance._overriden_type_descriptors.push_back(key);
        }
        return;
    }

    class CachedResult final {
    public:
        LayoutDescriptor layout_descriptor;
        bool invalidated;

        DANCE_UI_INLINE
        CachedResult(LayoutDescriptor &&layout_descriptor,
                     bool invalidated) noexcept :
        layout_descriptor(std::move(layout_descriptor)),
        invalidated(invalidated) {
        }

        DANCE_UI_INLINE
        CachedResult(CachedResult &&value) noexcept :
        layout_descriptor(std::move(value.layout_descriptor)),
        invalidated(value.invalidated) {
        }

        DANCE_UI_INLINE
        CachedResult &operator=(CachedResult &&value) noexcept {
            if (this != &value) {
                layout_descriptor = std::move(value.layout_descriptor);
                invalidated = std::move(value.invalidated);
            }
            return *this;
        }
    };

    class FetchResult final {
    public:
        const LayoutDescriptor &layout_descriptor;

        const bool is_hit_cache;
        const bool invalidated;

    private:

        friend class TypeDescriptorCache;

        DANCE_UI_INLINE
        FetchResult(const LayoutDescriptor &layout_descriptor,
                    bool is_hit_cache,
                    bool invalidated) noexcept :
        layout_descriptor(layout_descriptor),
        is_hit_cache(is_hit_cache),
        invalidated(invalidated) {
        }

        DANCE_UI_INLINE
        FetchResult(const FetchResult &value) noexcept :
        layout_descriptor(value.layout_descriptor),
        is_hit_cache(value.is_hit_cache),
        invalidated(value.invalidated) {
        }

        DANCE_UI_INLINE
        static FetchResult fromCache(const CachedResult &value) noexcept {
            FetchResult r = {
                value.layout_descriptor,
                true,
                value.invalidated
            };
            return r;
        }

        static FetchResult missingInCache(const CachedResult &value) noexcept {
            FetchResult r = {
                value.layout_descriptor,
                false,
                value.invalidated
            };
            return r;
        }

    };

    DANCE_UI_INLINE
    static
    FetchResult
    fetch(const Swift::metadata *metadata,
          const DanceUIComparisonMode comparisonMode,
          const LayoutDescriptor::CacheSlot slot) noexcept {
        return init_shared_cache().fetch(metadata, comparisonMode, LayoutDescriptor::HeapMode::HeapMode_0, slot);
    }

    friend bool LayoutDescriptor::compare_heap_objects(const swift::OpaqueValue *lhs, const swift::OpaqueValue *rhs, const DanceUIComparisonMode comparisonMode, bool is_capture_ref) noexcept;
    friend bool LayoutDescriptor::Builder::should_visit_fields(const Swift::metadata *metadata, bool using_equatable);

    FetchResult
    fetch(const Swift::metadata *metadata,
          const DanceUIComparisonMode comparisonMode,
          const LayoutDescriptor::HeapMode heapMode,
          const LayoutDescriptor::CacheSlot slot) noexcept {

#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        const uint64_t start_time = mach_absolute_time();
#endif
        static FetchResult invalidate_result = {
            LayoutDescriptor(metadata, LayoutDescriptor::LayoutBufferType()),
            false,
            true
        };

        const auto key = KeyType({metadata, {comparisonMode, heapMode}});
        {
            std::lock_guard<TypeDescriptorCache> lock(*this);
            const auto &it = find(key, true);
            if (it != _get_storage(slot).end()) {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
                const uint64_t end_time = mach_absolute_time();
                add_fetch(metadata, end_time - start_time,
                          false, false);
#endif
                return FetchResult::fromCache(it->second);
            } else if (slot == LayoutDescriptor::CacheSlot::FromCompareHeapObjectOrShouldVisit) {
                return invalidate_result;
            }
        }

        static EnvironmentVariable<bool, Debug::environment_async_layouts, true>
        async_layouts;
        if (!async_layouts ||
            (comparisonMode & DanceUIComparisonModeSynchronous) == DanceUIComparisonModeSynchronous) {
            {
                std::lock_guard<TypeDescriptorCache> lock(*this);
                if (BOOST_UNLIKELY(async_layouts)) {
                    const auto &it = find(key, false);
                    if (it != _get_storage(slot).end()) {
    #if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
                        const uint64_t end_time = mach_absolute_time();
                        add_fetch(metadata, end_time - start_time,
                                  false, false);
    #endif
                        return FetchResult::fromCache(it->second);
                    }
                }
            }
            bool invalidate = false;
            make_layout(metadata,
                        key.second,
                        key.third,
                        slot,
                        invalidate,
                        true );
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
            inc_make_layout_sync_count();
#endif
            auto &result = find(key, false)->second;
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
            const uint64_t end_time = mach_absolute_time();
            add_fetch(metadata, end_time - start_time,
                      false,
                      false);
#endif
            unlock();
            return FetchResult::missingInCache(result);
        }


        {
            std::lock_guard<TypeDescriptorCache> lock(*this);
            get_queued_requests().push_back({metadata, comparisonMode, heapMode, slot});
            if (!is_drain_queue()) {
                set_drain_queue(true);
                dispatch_async_f(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0),
                                 this,
                                 TypeDescriptorCache::drain_queue);
            }
        }
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        inc_query_invalidate_count();

        const uint64_t end_time = mach_absolute_time();
        add_fetch(metadata, end_time - start_time,
                  true, true);
#endif
        return invalidate_result;
    }

    DANCE_UI_INLINE
    void start_profiling() noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (!has_profile_data()) {
            return;
        }
        _profile_data = std::make_unique<ProfileData>();
#endif
    }

    DANCE_UI_INLINE
    void add_fetch(const Metadata *metadata,
                   uint64_t time,
                   bool asynchronous,
                   bool invalidate) const noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (!has_profile_data()) {
            return;
        }
        get_profile_data()->add_fetch(metadata, time, asynchronous, invalidate);
#endif
    }

    DANCE_UI_INLINE
    bool has_profile_data() const noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        return _profile_data != nullptr;
#else
        return false;
#endif
    }

    DANCE_UI_INLINE
    void inc_query_hit_count(void) const noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (!has_profile_data()) {
            return;
        }
        get_profile_data()->_inc_query_hit_count();
#endif
    }

    DANCE_UI_INLINE
    void inc_query_missing_count(void) const noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (!has_profile_data()) {
            return;
        }
        get_profile_data()->_inc_query_missing_count();
#endif
    }

    DANCE_UI_INLINE
    void inc_make_layout_sync_count(void) const noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (!has_profile_data()) {
            return;
        }
        get_profile_data()->_inc_make_layout_sync_count();
#endif
    }

    DANCE_UI_INLINE
    void inc_make_layout_async_count(void) const noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (!has_profile_data()) {
            return;
        }
        get_profile_data()->_inc_make_layout_async_count();
#endif
    }

    DANCE_UI_INLINE
    void inc_query_invalidate_count(void) const noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (!has_profile_data()) {
            return;
        }
        get_profile_data()->_inc_query_invalidate_count();
#endif
    }

#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
    DANCE_UI_INLINE
    const std::unique_ptr<ProfileData> &
    get_profile_data() const noexcept {
        return _profile_data;
    }
#endif

private:

    struct QueueEntry {
        const Swift::metadata *metadata;
        const DanceUIComparisonMode comparisonMode;
        const LayoutDescriptor::HeapMode heapMode;
        const LayoutDescriptor::CacheSlot slot;
    };

    static void drain_queue(void *context) noexcept {
        TypeDescriptorCache &instance = *reinterpret_cast<TypeDescriptorCache *>(context);
        instance.lock();

        auto &requests = instance.get_queued_requests();
        while (!requests.empty()) {
            const auto &request = requests.back();

            const auto key = KeyType({request.metadata, {request.comparisonMode, request.heapMode}});

            auto it = instance.find(key, false);
            if (it == instance._get_storage(request.slot).end()) {
                instance.unlock();
                bool invalidate = false;
                instance.make_layout(key.first,
                                     key.second,
                                     key.third,
                                     request.slot,
                                     invalidate,
                                     true );
                instance.inc_make_layout_async_count();
            }
            requests.pop_back();
        }

        instance.set_drain_queue(false);
        instance.unlock();
    }

    using KeyType = TypeDescriptorKey;

    static TypeDescriptorCache &init_shared_cache() noexcept {


        static dispatch_once_t shared_once;
        dispatch_once(&shared_once, ^{
            TypeDescriptorCache::_shared_cache = new TypeDescriptorCache();
        });
        return *TypeDescriptorCache::_shared_cache;
    }

    TypeDescriptorCache() noexcept {
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        static EnvironmentVariable<bool, Debug::environment_profiling_async_layouts, false>
        profiling_async_layouts;
        if (profiling_async_layouts) {
            start_profiling();
        }
#endif
    }

    ~TypeDescriptorCache() noexcept {
#if DEBUG
        fatalError(0, "TypeDescriptorCache should never be deallocated!");
#endif
    }

    TypeDescriptorCache(const TypeDescriptorCache &) = delete;
    TypeDescriptorCache(TypeDescriptorCache &) = delete;

    DANCE_UI_INLINE
    void make_layout(const Swift::metadata *metadata,
                     DanceUIComparisonMode comparisonMode,
                     LayoutDescriptor::HeapMode heapMode,
                     LayoutDescriptor::CacheSlot slot,
                     bool &invalidate,
                     bool acquire_lock_after_make = true) noexcept {
        auto layout = LayoutDescriptor::make_layout(metadata,
                                                    DanceUIComparisonModeGetComparison(comparisonMode),
                                                    heapMode,
                                                    invalidate);
        const auto key = KeyType({metadata, {comparisonMode, heapMode}});
        if (BOOST_LIKELY(acquire_lock_after_make)) {
            lock();
        }
        std::pair<const KeyType, CachedResult> vt(key,
                                                  CachedResult(LayoutDescriptor(metadata, std::move(layout)), invalidate));
        _get_storage(slot).insert(std::move(vt));
#if DEBUG
        static EnvironmentVariable<bool, Debug::environment_print_layouts, false>
        print_layouts;
        if (print_layouts) {
            auto found = _get_storage(slot).find(key);
            if (found != _get_storage(slot).end() && found->second.invalidated == false) {
                found->second.layout_descriptor.dump();
            }
        }
#endif
    }

    friend class std::lock_guard<TypeDescriptorCache>;

    DANCE_UI_INLINE
    void lock(void) noexcept {
        os_unfair_lock_lock(&_lock);
    }

    DANCE_UI_INLINE
    void unlock(void) noexcept {
        os_unfair_lock_unlock(&_lock);
    }

    DANCE_UI_INLINE
    std::unordered_map<KeyType, CachedResult>::const_iterator
    find(const KeyType &key, const LayoutDescriptor::CacheSlot slot, bool profiling) const noexcept {
        auto it = _get_storage(slot).find(key);

#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (BOOST_LIKELY(profiling)) {
            if (BOOST_UNLIKELY(it == _get_storage(slot).end())) {
                inc_query_missing_count();
            } else {
                inc_query_hit_count();
                if (BOOST_UNLIKELY(it->second.invalidated)) {
                    inc_query_invalidate_count();
                }
            }
        }
#endif
        return it;
    }

    DANCE_UI_INLINE
    std::unordered_map<KeyType, CachedResult>::iterator
    find(const KeyType &key, bool profiling) noexcept {
        const LayoutDescriptor::CacheSlot slot = LayoutDescriptor::CacheSlot::Default;
        auto it = _get_storage(slot).find(key);

#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
        if (BOOST_LIKELY(profiling)) {
            if (BOOST_UNLIKELY(it == _get_storage(slot).end())) {
                inc_query_missing_count();
            } else {
                inc_query_hit_count();
                if (BOOST_UNLIKELY(it->second.invalidated)) {
                    inc_query_invalidate_count();
                }
            }
        }
#endif
        return it;
    }

    DANCE_UI_INLINE
    bool is_drain_queue(void) const noexcept {
        return _is_drain_queue;
    }

    DANCE_UI_INLINE
    void set_drain_queue(bool value) noexcept {
        _is_drain_queue = value;
    }

    DANCE_UI_INLINE
    vector<QueueEntry, 0, uint64_t> &
    get_queued_requests() noexcept {
        return _queued_requests;
    }

    DANCE_UI_INLINE
    const vector<QueueEntry, 0, uint64_t> &
    get_queued_requests() const noexcept {
        return _queued_requests;
    }

    const std::unordered_map<KeyType, CachedResult> &
    _get_storage(LayoutDescriptor::CacheSlot slot) const noexcept {
        return _storage;
    }

    std::unordered_map<KeyType, CachedResult> &
    _get_storage(LayoutDescriptor::CacheSlot slot) noexcept {
        assert(os_unfair_lock_trylock(&_lock) == false);
        return _storage;
    }

private:

    static TypeDescriptorCache *_shared_cache;

    os_unfair_lock _lock = OS_UNFAIR_LOCK_INIT;
    std::unordered_map<KeyType, CachedResult> _storage;
    vector<QueueEntry, 0, uint64_t> _queued_requests;
    vector<std::pair<const void *, DanceUIComparisonMode>, 0, uint32_t> _overriden_type_descriptors;

    bool _is_drain_queue = false;
#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
    mutable std::unique_ptr<ProfileData> _profile_data;
#endif
};

TypeDescriptorCache *TypeDescriptorCache::_shared_cache = nullptr;

#if defined(__DANCE_UI_RUNTIME_ENABLE_PROFILING__)
extern "C"
void _DGLayoutProfilingPrint(void) {
    std::ostringstream ss;
    TypeDescriptorCache::print(ss);
    std::cout << ss.str();
}
#endif

struct Visitor {

private:
    friend class FrameRAII;

    class FrameRAII final {
    public:
        DANCE_UI_INLINE
        FrameRAII(const Visitor &visitor) noexcept :
        visitor(visitor) {
            visitor.onEnterItem();
        }

        DANCE_UI_INLINE
        ~FrameRAII() noexcept {
            visitor.onLeaveItem();
        }
    private:
        const Visitor &visitor;
    };

public:

    mutable const Metadata *currentType;

    std::ostream &os;
    mutable size_t indent = 0;

    DANCE_UI_INLINE
    Visitor(std::ostream &os) :
    currentType(nullptr),
    os(os),
    indent(0) {
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::DataItem &item) const noexcept {
        FrameRAII frame(*this);
        os << "(" << (item.skip ? "skip" : "read") << " :#length " << item.instance_size << ")";
        return true;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::EqualsItem &item) const noexcept {
        FrameRAII frame(*this);
        os << "(== " << ":#type " << type_name_for_item(item.type) << " :#length " << item.instance_size << ")";
        return true;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::IndirectItem &item) const noexcept {
        FrameRAII frame(*this);
        os << "(indirect " << ":#type " << type_name_for_item(item.type) << " :#length " << item.instance_size << ")";
        return true;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::ExistentialItem &item) const noexcept {
        FrameRAII frame(*this);
        os << "(existential " << ":#type " << type_name_for_item(item.type) << " :#length " << item.instance_size << ")";
        return true;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::HeapRefItem &item) const noexcept {
        FrameRAII frame(*this);
        os << "(" << (item.isHeapLocalVariable ? "capture-ref" : "heap-ref") << " :#length " << item.instance_size << ")";
        return true;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::ReferenceItem &item, uint32_t case_index) const noexcept {
        printIndent();
        os << "(case " << case_index;
        for (auto &bufferItem: item.buffer) {
            indent += 1;
            boost::apply_visitor(*this, bufferItem);
            indent -= 1;
        }
        os << ")";
        return true;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::EnumItem &item) const noexcept {
        FrameRAII frame(*this);
        os << "(enum " << ":#type " << type_name_for_item(item.type) <<
        " :#length " << item.instance_size << "\n";
        for (uint32_t case_idx = 0; case_idx < item.cases.size(); case_idx++) {
            const LayoutDescriptor::Builder::ReferenceItem &case_element = item.cases[case_idx];
            operator()(case_element, case_idx);
            if (case_idx < (item.cases.size() - 1)) {
                os << "\n";
            }
        }
        os << ")";
        return true;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::NestedItem &item) const noexcept {
        FrameRAII frame(*this);
        os << "(nested " << ":#type " << type_name_for_item(item.get_type()) <<
        " :#length " << item.instance_size <<
        " :#address " << std::hex << item.layout_descriptor << std::dec <<
        ")";
        return true;
    }

    DANCE_UI_INLINE
    const char *type_name_for_item(const Metadata *type) const noexcept {
        return reinterpret_cast<const Swift::metadata *>(type)->name(false).data;
    }

    DANCE_UI_INLINE
    void printIndent() const noexcept {
        for (size_t level = 0; level < indent; level ++) {
            os << "    ";
        }
    }

private:

    DANCE_UI_INLINE
    void onEnterItem() const noexcept {
        if (indent > 0) {
            os << "\n";
        }
        printIndent();

        indent += 1;
    }

    DANCE_UI_INLINE
    void onLeaveItem() const noexcept {
        indent -= 1;
        if (indent == 0) {
            os << "\n";
        }
    }
};

struct ComparisonVisitor {

    struct EnumContext {
        const Metadata *enumType;
        const Metadata *caseType;
        uint32_t caseIndex;
    };

    const LayoutDescriptor &descriptor;

    const DanceUIComparisonMode comparisonMode;

    const LayoutDescriptor::HeapMode heapMode;

    const OpaqueValue *lhs;

    const OpaqueValue *rhs;

    mutable const Metadata *currentType;

    DANCE_UI_INLINE
    ComparisonVisitor(const LayoutDescriptor &descriptor,
                      const DanceUIComparisonMode comparisonMode,
                      const LayoutDescriptor::HeapMode heapMode,
                      const OpaqueValue *lhs,
                      const OpaqueValue *rhs) :
    descriptor(descriptor),
    comparisonMode(comparisonMode),
    heapMode(heapMode),
    lhs(lhs), rhs(rhs),
    currentType(nullptr) {
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::DataItem &item) const {
        if (item.instance_size == 0) {
            return true;
        }
        if (item.skip) {
            return true;
        }
        auto projectionLHS = projection(lhs, item);
        auto projectionRHS = projection(rhs, item);
        const bool result = memcmp(projectionLHS,
                                   projectionRHS,
                                   item.instance_size) == 0;
        return result;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::EqualsItem &item) const {
        const OpaqueValue *projectionLHS = projection(lhs, item);
        const OpaqueValue *projectionRHS = projection(rhs, item);
        if (projectionLHS == projectionRHS) {
            return true;
        }

        if (item.equatable == nullptr) {
            return false;
        }

        return DanceUI_DispatchEquatable(projectionLHS, projectionRHS,
                                         item.type, item.equatable);
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::IndirectItem &item) const {
        const OpaqueValue *projectionLHS = projection(lhs, item);
        const OpaqueValue *projectionRHS = projection(rhs, item);
        if (projectionLHS == projectionRHS) {
            return true;
        }
        return descriptor.compare_indirect(item.type->getTypeLayout(),
                                           dyn_cast<EnumMetadata>(currentType),
                                           item.type,
                                           comparisonMode,
                                           0,
                                           item.instance_size,
                                           projectionLHS, projectionRHS);
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::ExistentialItem &item) const {
        const OpaqueValue *projectionLHS = projection(lhs, item);
        const OpaqueValue *projectionRHS = projection(rhs, item);
        return LayoutDescriptor::compare_existential_values(item.type,
                                                            projectionLHS,
                                                            projectionRHS,
                                                            comparisonMode);
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::HeapRefItem &item) const {


        const swift::OpaqueValue *projectionLHS = projection(lhs, item);
        const swift::OpaqueValue *projectionRHS = projection(rhs, item);

        const swift::HeapObject *lhsHeapObject = *(reinterpret_cast<const swift::HeapObject **>(reinterpret_cast<uintptr_t>(projectionLHS)));
        const swift::HeapObject *rhsHeapObject = *(reinterpret_cast<const swift::HeapObject **>(reinterpret_cast<uintptr_t>(projectionRHS)));

        if (lhsHeapObject == rhsHeapObject) {
            return true;
        }

        return LayoutDescriptor::compare_heap_objects(projectionLHS,
                                                      projectionRHS,
                                                      comparisonMode,
                                                      item.isHeapLocalVariable);
    }

    bool getEnumContext(EnumContext &ctx,
                        const LayoutDescriptor::Builder::EnumItem &item) const {
        ctx.enumType = item.type;

        auto enumType = cast<EnumMetadata>(item.type);
        auto description = enumType->getDescription();

        auto caseIndex = 0;
        const auto payloadCasesCount = description->getNumPayloadCases();
        const auto casesCount = description->getNumCases();

        const OpaqueValue *projectionLHS = projection(lhs, item);
        const OpaqueValue *projectionRHS = projection(rhs, item);

        if (payloadCasesCount == 1 && casesCount == 1) {
            auto valueWitnessTable =
            static_cast<const EnumValueWitnessTable *>(enumType->getValueWitnesses());
            const auto lhsTag = valueWitnessTable->getEnumTagSinglePayload(projectionLHS, description->getNumEmptyCases(), enumType);
            const auto rhsTag = valueWitnessTable->getEnumTagSinglePayload(projectionRHS, description->getNumEmptyCases(), enumType);
            if (lhsTag != rhsTag) {
                return false;
            }
            caseIndex = lhsTag;
        } else {
            auto valueWitnessTable = static_cast<const EnumValueWitnessTable *>(enumType->getValueWitnesses());
            const auto lhsTag = valueWitnessTable->getEnumTag(projectionLHS, enumType);
            const auto rhsTag = valueWitnessTable->getEnumTag(projectionRHS, enumType);
            if (lhsTag != rhsTag) {
                return false;
            }
            caseIndex = lhsTag;
        }


        ctx.caseIndex = caseIndex;
        return true;
    }

    DANCE_UI_INLINE
    bool operator()(const LayoutDescriptor::Builder::EnumItem &item) const {
        if (item.cases.size() == 0) {
            return true;
        }
        EnumContext enumContext;
        if (!getEnumContext(enumContext, item)) {
            return false;
        }

        using size_type = decltype(item.cases)::size_type;
        size_type caseIndex = enumContext.caseIndex;
        if (caseIndex >= item.cases.size()) {


            bool foundInBuffer = false;
            for (size_type idx = 0; idx < item.cases.size(); idx ++) {
                if (item.cases[idx].offset == caseIndex) {
                    caseIndex = idx;
                    foundInBuffer = true;
                    break;
                }
            }
            if (!foundInBuffer) {


                return true;
            }
        }
        const Metadata *storedCurrentMetadata = currentType;
        currentType = enumContext.enumType;
        for (const auto &item : item.cases[caseIndex].buffer) {
            if (!boost::apply_visitor(*this, item)) {
                currentType = storedCurrentMetadata;
                return false;
            }
        }
        currentType = storedCurrentMetadata;
        return true;
    }

    bool operator()(const LayoutDescriptor::Builder::NestedItem &item) const {
        const swift::OpaqueValue *projectionLHS = projection(lhs, item);
        const swift::OpaqueValue *projectionRHS = projection(rhs, item);

        return item.layout_descriptor->compare(projectionLHS, projectionRHS, comparisonMode, heapMode);
    }
};

struct PartialComparisonVisitor : public ComparisonVisitor {

    const size_t skipBytes;
    const size_t compareBytes;

    mutable size_t comparedBytes;

    enum State: size_t {
        Skip = 0x10,
        Inner = 0x20,
        Matched = 0x40,
        End = 0x80,
    };

    static constexpr const size_t ValueMask = 0xF;
    static constexpr const size_t StateMask = 0xF0;

    DANCE_UI_INLINE constexpr
    static uint8_t getStateValue(const size_t state) {
        return (state & ValueMask);
    }

    DANCE_UI_INLINE constexpr
    static State getState(const size_t state) {
        return State(state & StateMask);
    }

    DANCE_UI_INLINE
    PartialComparisonVisitor(const LayoutDescriptor &descriptor,
                             const DanceUIComparisonMode comparisonMode,
                             const LayoutDescriptor::HeapMode heapMode,
                             const size_t skipBytes,
                             const size_t compareBytes,
                             const OpaqueValue *lhs,
                             const OpaqueValue *rhs) :
    ComparisonVisitor(descriptor, comparisonMode, heapMode, lhs, rhs),
    skipBytes(skipBytes),
    compareBytes(compareBytes),
    comparedBytes(0) {
    }

    DANCE_UI_INLINE
    size_t matchItemState(const LayoutDescriptor::Builder::RangeItem &item) const {
        if (BOOST_UNLIKELY(compareBytes == comparedBytes)) {
            return size_t(State::End | true);
        }

        const uint64_t &item_lb = item.start_offset;
        const uint64_t &item_length = item.instance_size;

        const uint64_t &scan_lb = comparedBytes + skipBytes;
        const uint64_t &scan_length = compareBytes;

        if (BOOST_UNLIKELY(item_lb == scan_lb && item_length <= scan_length)) {
            return State::Matched;
        }
        if (BOOST_LIKELY((item_lb + item_length) <= scan_lb ||
                         (scan_lb + scan_length) <= item_lb)) {
            return State::Skip;
        }
        return State::Inner;
    }

    DANCE_UI_INLINE
    void compute_new_item(const State &state,
                          const LayoutDescriptor::Builder::RangeItem &item,
                          size_t &new_instance_offset,
                          size_t &new_instance_size,
                          size_t &skip_size,
                          bool &skip) const noexcept {
        new_instance_offset = 0;
        new_instance_size = 0;
        skip_size = 0;
        skip = false;

        if (state != State::Inner) {
            return;
        }

        const auto offset = skipBytes - item.start_offset;
        new_instance_offset = item.start_offset + offset;
        new_instance_size = std::min(item.instance_size - offset, compareBytes - comparedBytes);
        skip_size = 0;
        skip = false;
    }

    DANCE_UI_INLINE
    size_t operator()(const LayoutDescriptor::Builder::DataItem &item) const {
        const size_t stateValue = matchItemState(item);
        const State state = getState(stateValue);
        switch (state) {
            case State::Skip: BOOST_FALLTHROUGH;
            case State::End:
                return state;
            case State::Matched: {
                const bool result = ComparisonVisitor::operator()(item);
                comparedBytes += (item.instance_size + item.skip_size);
                return size_t(State::Matched | result);
            }
            case State::Inner: {
                LayoutDescriptor::Builder::DataItem newItem;
                compute_new_item(state, item, newItem.start_offset, newItem.instance_size, newItem.skip_size, newItem.skip);
                const bool result = ComparisonVisitor::operator()(newItem);
                comparedBytes += (newItem.instance_size);
                return size_t(State::Matched | result);
            }
        }
    }

    DANCE_UI_INLINE
    size_t operator()(const LayoutDescriptor::Builder::EqualsItem &item) const {
        const size_t stateValue = matchItemState(item);
        const State state = getState(stateValue);
        switch (state) {
            case State::Skip: BOOST_FALLTHROUGH;
            case State::End:
                return state;
            case State::Matched: {
                const bool result = ComparisonVisitor::operator()(item);
                comparedBytes += (item.instance_size + item.skip_size);
                return size_t(State::Matched | result);
            }
            case State::Inner: {
                LayoutDescriptor::Builder::DataItem newItem;
                compute_new_item(state, item, newItem.start_offset, newItem.instance_size, newItem.skip_size, newItem.skip);
                const bool result = ComparisonVisitor::operator()(newItem);
                comparedBytes += (newItem.instance_size);

                return size_t(State::Matched | result);
            }
        }
    }

    DANCE_UI_INLINE
    size_t operator()(const LayoutDescriptor::Builder::IndirectItem &item) const {
        const size_t stateValue = matchItemState(item);
        const State state = getState(stateValue);
        switch (state) {
            case State::Skip: BOOST_FALLTHROUGH;
            case State::End:
                return state;
            case State::Matched: {
                const bool result = ComparisonVisitor::operator()(item);
                comparedBytes += (item.instance_size + item.skip_size);
                return size_t(State::Matched | result);
            }
            case State::Inner: {
                const OpaqueValue *projectionLHS = projection(lhs, item);
                const OpaqueValue *projectionRHS = projection(rhs, item);
                if (projectionLHS == projectionRHS) {
                    return size_t(State::Matched | true);
                }
                comparedBytes += (item.instance_size + item.skip_size);
                const bool result = descriptor.compare_indirect(item.type->getTypeLayout(),
                                                                dyn_cast<EnumMetadata>(currentType),
                                                                item.type,
                                                                comparisonMode,
                                                                0,
                                                                item.instance_size,
                                                                projectionLHS, projectionRHS);
                return size_t(State::Matched | result);
            }
        }
    }

    DANCE_UI_INLINE
    size_t operator()(const LayoutDescriptor::Builder::ExistentialItem &item) const {
        const size_t stateValue = matchItemState(item);
        const State state = getState(stateValue);
        switch (state) {
            case State::Skip: BOOST_FALLTHROUGH;
            case State::End:
                return state;
            case State::Matched: {
                const bool result = ComparisonVisitor::operator()(item);
                comparedBytes += (item.instance_size + item.skip_size);
                return size_t(State::Matched | result);
            }
            case State::Inner: {
                const Metadata *type = descriptor.get_type();


                const Metadata *lhs_type = reinterpret_cast<const Swift::existential_type_metadata *>(type)->dynamic_type(lhs);
                if (lhs_type == nullptr) {
                    return size_t(State::Matched | false);
                }
                const Metadata *rhs_type = reinterpret_cast<const Swift::existential_type_metadata *>(type)->dynamic_type(rhs);
                if (rhs_type != lhs_type) {
                    return size_t(State::Matched | false);
                }
                const OpaqueValue *lhs_value = reinterpret_cast<const Swift::existential_type_metadata *>(type)->project_value(lhs);
                const OpaqueValue *rhs_value = reinterpret_cast<const Swift::existential_type_metadata *>(type)->project_value(rhs);
                if (lhs_value == rhs_value) {
                    return size_t(State::Matched | true);
                }

                const OpaqueValue *projectionLHS = projection(lhs_value, item);
                const OpaqueValue *projectionRHS = projection(rhs_value, item);
                if (projectionLHS == projectionRHS) {
                    return size_t(State::Matched | true);
                }
                const size_t length = compareBytes - comparedBytes;
                comparedBytes += (item.instance_size + item.skip_size);


                const TypeDescriptorCache::FetchResult fetch_result = TypeDescriptorCache::fetch(reinterpret_cast<const Swift::metadata *>(lhs_type), comparisonMode, LayoutDescriptor::CacheSlot::Default);
                if (fetch_result.invalidated) {
                    return size_t(State::Matched | compare_bytes(projectionLHS, projectionRHS, length));
                }

                const bool result = fetch_result.layout_descriptor.compare(projectionLHS, projectionRHS, comparisonMode, heapMode);
                return size_t(State::Matched | result);
            }
        }
    }

    DANCE_UI_INLINE
    size_t operator()(const LayoutDescriptor::Builder::HeapRefItem &item) const {
        const size_t stateValue = matchItemState(item);
        const State state = getState(stateValue);
        switch (state) {
            case State::Skip: BOOST_FALLTHROUGH;
            case State::End:
                return state;
            case State::Matched: {
                const bool result = ComparisonVisitor::operator()(item);
                comparedBytes += (item.instance_size + item.skip_size);
                return size_t(State::Matched | result);
            }
            case State::Inner: {
                comparedBytes += (item.instance_size + item.skip_size);
                return size_t(State::Matched | false);
            }
        }
    }

    DANCE_UI_INLINE
    size_t operator()(const LayoutDescriptor::Builder::EnumItem &item) const {
        const size_t stateValue = matchItemState(item);
        const State state = getState(stateValue);
        switch (state) {
            case State::Skip: BOOST_FALLTHROUGH;
            case State::End:
                return state;
            case State::Matched: {
                const bool result = ComparisonVisitor::operator()(item);
                comparedBytes += (item.instance_size + item.skip_size);
                return size_t(State::Matched | result);
            }
            case State::Inner: {
                EnumContext enumContext;
                if (!getEnumContext(enumContext, item)) {
                    return size_t(State::Matched | false);
                }

                using size_type = decltype(item.cases)::size_type;
                size_type caseIndex = enumContext.caseIndex;
                if (caseIndex >= item.cases.size()) {


                    bool foundInBuffer = false;
                    for (size_type idx = 0; idx < item.cases.size(); idx ++) {
                        if (item.cases[idx].offset == caseIndex) {
                            caseIndex = idx;
                            foundInBuffer = true;
                            break;
                        }
                    }
                    if (!foundInBuffer) {


                        return size_t(State::Matched | true);
                    }
                }

                bool result = false;
                const Metadata *storedCurrentMetadata = currentType;
                currentType = enumContext.enumType;

                for (auto &item : item.cases[caseIndex].buffer) {
                    const auto stateRawValue = boost::apply_visitor(*this, item);
                    const PartialComparisonVisitor::State state =
                    PartialComparisonVisitor::getState(stateRawValue);
                    switch (state) {
                        case PartialComparisonVisitor::State::Matched: {
                            result = bool(PartialComparisonVisitor::getStateValue(stateRawValue));
                            if (result && (comparedBytes < compareBytes)) {
                                continue;
                            }
                            break;
                        }
                        case PartialComparisonVisitor::State::Inner: BOOST_FALLTHROUGH;
                        case PartialComparisonVisitor::State::Skip:
                            continue;
                        case PartialComparisonVisitor::State::End: BOOST_FALLTHROUGH;
                        default:
                            break;
                    }
                }

                currentType = storedCurrentMetadata;
                return size_t(State::Matched | result);
            }
        }
    }

    bool operator()(const LayoutDescriptor::Builder::NestedItem &item) const {
        const swift::OpaqueValue *projectionLHS = projection(lhs, item);
        const swift::OpaqueValue *projectionRHS = projection(rhs, item);

        return item.layout_descriptor->compare_partial(projectionLHS, projectionRHS,
                                                       skipBytes + comparedBytes,
                                                       compareBytes,
                                                       comparisonMode, heapMode);
    }
};

}

const LayoutDescriptor &
LayoutDescriptor::fetch(const Swift::metadata *metadata,
                        DanceUIComparisonMode comparisonMode,
                        LayoutDescriptor::CacheSlot slot) noexcept {
    const auto &result = TypeDescriptorCache::fetch(metadata, comparisonMode, slot);
    if (result.invalidated) {
        return LayoutDescriptor::get_invalid();
    }
    return result.layout_descriptor;
}

bool
LayoutDescriptor::compare(const OpaqueValue *lhs,
                          const OpaqueValue *rhs,
                          const DanceUIComparisonMode comparisonMode,
                          const HeapMode heapMode) const noexcept {
    if (lhs == rhs) {
        return true;
    }
    if (LayoutDescriptor::get_invalid() == *this) {
        return false;
    }

    ComparisonVisitor visitor(*this, comparisonMode, heapMode, lhs, rhs);
    uint32_t current_item_idx __unused = 0;

    for (auto &item : _buffer) {
        if (!boost::apply_visitor(visitor, item)) {
            return false;
        }
        current_item_idx += 1;
    }
    return true;
}

bool
LayoutDescriptor::compare_partial(const OpaqueValue *lhs,
                                  const OpaqueValue *rhs,
                                  size_t skipBytes,
                                  size_t compareBytes,
                                  const DanceUIComparisonMode comparisonMode,
                                  const HeapMode heapMode) const noexcept {
    if (lhs == rhs) {
        return true;
    }


    const PartialComparisonVisitor visitor(*this, comparisonMode, heapMode,
                                           skipBytes, compareBytes, lhs, rhs);

    uint32_t current_item_idx = 0;
    for (auto &item : _buffer) {
        const auto stateRawValue = boost::apply_visitor(visitor, item);
        const PartialComparisonVisitor::State state =
        PartialComparisonVisitor::getState(stateRawValue);
        switch (state) {
            case PartialComparisonVisitor::State::Matched:
                if (!bool(PartialComparisonVisitor::getStateValue(stateRawValue))) {
                    return false;
                }
                if (visitor.comparedBytes >= visitor.compareBytes) {


                    return true;
                }
                current_item_idx += 1;
                continue;
            case PartialComparisonVisitor::State::Inner: BOOST_FALLTHROUGH;
            case PartialComparisonVisitor::State::Skip:
                current_item_idx += 1;
                continue;
            case PartialComparisonVisitor::State::End: BOOST_FALLTHROUGH;
            default:
                break;
        }
    }
    return false;
}

bool
LayoutDescriptor::compare_indirect(const TypeLayout *valueLayout,
                                   const EnumMetadata *enumType,
                                   const Metadata *type,
                                   DanceUIComparisonMode comparisonMode,
                                   size_t skipBytes,
                                   size_t compareBytes,
                                   const OpaqueValue *lhs,
                                   const OpaqueValue *rhs) const noexcept {
    auto enumValueWitnessTable =
    static_cast<const EnumValueWitnessTable *>(enumType->getValueWitnesses());

    const auto size = enumValueWitnessTable->size;
    const auto bufferSize = ((enumValueWitnessTable->size + 0xf) & ~0xf);
    OpaqueValue *lhsBuffer = static_cast<OpaqueValue *>(alloca(bufferSize));
    OpaqueValue *rhsBuffer = static_cast<OpaqueValue *>(alloca(bufferSize));

    memcpy(lhsBuffer, lhs, size);
    memcpy(rhsBuffer, rhs, size);

    enumValueWitnessTable->destructiveProjectEnumData(lhsBuffer, enumType);
    enumValueWitnessTable->destructiveProjectEnumData(rhsBuffer, enumType);

    HeapObject *lhsHeapObject = reinterpret_cast<HeapObject *>(*(uint64_t *)lhsBuffer);
    HeapObject *rhsHeapObject = reinterpret_cast<HeapObject *>(*(uint64_t *)rhsBuffer);

    if (lhsHeapObject == rhsHeapObject) {
        return true;
    }

    const OpaqueValue *lhsValue = swift_projectBox(lhsHeapObject);
    const OpaqueValue *rhsValue = swift_projectBox(rhsHeapObject);


    const LayoutDescriptor &layoutDescriptor =
    LayoutDescriptor::fetch(reinterpret_cast<const Swift::metadata *>(type), comparisonMode);
    const auto result = skipBytes ?
    layoutDescriptor.compare(lhsValue,
                             rhsValue,
                             comparisonMode,
                             HeapMode::HeapMode_0) :
    layoutDescriptor.compare_partial(lhsValue,
                                     rhsValue,
                                     skipBytes,
                                     compareBytes & ~DanceUIComparisonModeAsynchronous,
                                     comparisonMode,
                                     HeapMode::HeapMode_0);
    return result;
}

bool LayoutDescriptor::compare_heap_objects(const swift::OpaqueValue *lhs,
                                            const swift::OpaqueValue *rhs,
                                            const DanceUIComparisonMode comparisonMode,
                                            bool is_capture_ref) noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    if (lhs == rhs) {
        return true;
    }
    if (lhs != nullptr && rhs != nullptr) {
        const swift::HeapObject *projectionLHS = *(reinterpret_cast<const swift::HeapObject **>(reinterpret_cast<uintptr_t>(lhs)));
        const swift::HeapObject *projectionRHS = *(reinterpret_cast<const swift::HeapObject **>(reinterpret_cast<uintptr_t>(rhs)));
        if (projectionLHS != nullptr && projectionRHS != nullptr) {
            if (projectionLHS->metadata != projectionRHS->metadata) {
                return false;
            }

            const HeapMode heapMode = is_capture_ref ? HeapMode::HeapMode_2 : HeapMode::HeapMode_1;
            TypeDescriptorCache &cache = TypeDescriptorCache::init_shared_cache();
            const auto &result = cache.fetch(reinterpret_cast<const Swift::metadata *>(projectionLHS->metadata),
                                             comparisonMode,
                                             heapMode,
                                             LayoutDescriptor::CacheSlot::FromCompareHeapObjectOrShouldVisit);
            if (result.invalidated == true) {
                return false;
            }
            const LayoutDescriptor &layout_descriptor = result.layout_descriptor;
            return layout_descriptor.compare(reinterpret_cast<const swift::OpaqueValue *>(projectionLHS),
                                             reinterpret_cast<const swift::OpaqueValue *>(projectionRHS),
                                             (comparisonMode & ~DanceUIComparisonModeAsynchronous),
                                             heapMode);
        }
    }
    return false;
}

bool
LayoutDescriptor::compare_existential_values(const swift::Metadata *type,
                                             const OpaqueValue *lhs,
                                             const OpaqueValue *rhs,
                                             const DanceUIComparisonMode comparisonMode) noexcept {
    const Metadata *lhs_type = reinterpret_cast<const Swift::existential_type_metadata *>(type)->dynamic_type(lhs);
    if (lhs_type == nullptr) {
        return false;
    }
    const Metadata *rhs_type = reinterpret_cast<const Swift::existential_type_metadata *>(type)->dynamic_type(rhs);
    if (rhs_type != lhs_type) {
        return false;
    }
    const OpaqueValue *lhs_value = reinterpret_cast<const Swift::existential_type_metadata *>(type)->project_value(lhs);
    const OpaqueValue *rhs_value = reinterpret_cast<const Swift::existential_type_metadata *>(type)->project_value(rhs);
    if (lhs_value == rhs_value) {
        return true;
    }

    DanceUIComparisonMode adjustedComparisonMode = comparisonMode;
    if (lhs_value != lhs || rhs_value != rhs) {
        adjustedComparisonMode = (adjustedComparisonMode & ~DanceUIComparisonModeAsynchronous);
    }


    const LayoutDescriptor &layout_descriptor = LayoutDescriptor::fetch(reinterpret_cast<const Swift::metadata *>(lhs_type),
                                                                        adjustedComparisonMode);
    if (layout_descriptor == LayoutDescriptor::get_invalid()) {
        const size_t value_size = lhs_type->vw_size();
        return compare_bytes(lhs_value, rhs_value, value_size);
    }

    return layout_descriptor.compare(lhs_value, rhs_value, comparisonMode, HeapMode::HeapMode_0);
}

void
LayoutDescriptor::dump() const noexcept {
    std::ostringstream ss;
    dump(ss);
    printf("%s", ss.str().data());
}

void
LayoutDescriptor::dump(std::ostream &os) const noexcept {
    Visitor printer(os);
    os << "== " << reinterpret_cast<const Swift::metadata *>(get_type())->name(false).data << " == \n";
    printer.os << "(layout" <<
    " :#address " << std::hex << this << std::dec;
    printer.indent += 1;
    for (auto &item : _buffer) {
        boost::apply_visitor(printer, item);
    }
    printer.indent -= 1;
    printer.os << ")\n";
}

const LayoutDescriptor &
LayoutDescriptor::get_invalid() noexcept {
    static const LayoutDescriptor invalid = LayoutDescriptor(nullptr, LayoutDescriptor::LayoutBufferType());
    return invalid;
}

void
LayoutDescriptor::add_type_descriptor_override(const void *type_descriptor,
                                               DanceUIComparisonMode comparison_mode) {
    return TypeDescriptorCache::add_type_descriptor_override(type_descriptor, comparison_mode);
}

LayoutDescriptor::LayoutBufferType
LayoutDescriptor::make_layout(const Swift::metadata *metadata,
                              DanceUIComparisonMode _comparisonMode,
                              HeapMode heapMode,
                              bool &invalidate) noexcept {
    const DanceUIComparisonMode comparisonMode = mode_for_type(metadata, _comparisonMode);
    LayoutDescriptor::Builder builder(comparisonMode, heapMode);
    invalidate = false;
    switch (heapMode) {
        case HeapMode::HeapMode_0:
        case HeapMode::HeapMode_1: {

            const Metadata *type = metadata->getType();
            const auto valueWitnessTable = type->getValueWitnesses();
            const bool isPOD = valueWitnessTable->isPOD();
            const DanceUIComparisonMode derivedComparisonMode = (isPOD ? DanceUIComparisonModePod : DanceUIComparisonModeDefault) | DanceUIComparisonModeInline;


            if (can_use_equatable(comparisonMode, derivedComparisonMode)) {

                if (const auto witnessTable = metadata->equatable()) {
                    builder.push_back(Builder::EqualsItem(0,
                                                          type->vw_size(),
                                                          0,
                                                          type,
                                                          witnessTable));
                    break;
                }
            }
            if (heapMode == HeapMode::HeapMode_0) {


                if (!metadata->visit(builder)) {

                    invalidate = true;
                    return LayoutBufferType();
                }
            } else {


                if (!metadata->visit_heap(builder,
                                          (uint32_t(HeapType::Class) |
                                           uint32_t(HeapType::GenericLocalVariable)))) {

                    invalidate = true;
                    return LayoutBufferType();
                }
            }
            break;
        }
        case HeapMode::HeapMode_2: {


            if (!metadata->visit_heap(builder, uint32_t(HeapType::LocalVariable))) {

                invalidate = true;
                return LayoutBufferType();
            }
            break;
        }
    }


    return builder.commit(metadata);
}

DanceUIComparisonMode LayoutDescriptor::mode_for_type(const Swift::metadata *metadata,
                                                      DanceUIComparisonMode comparison_mode) {
    if (metadata == nullptr) {
        return comparison_mode;
    }
    const TypeContextDescriptor *descriptor = metadata->descriptor();
    if (descriptor == nullptr) {
        return comparison_mode;
    }
    return TypeDescriptorCache::fetch_mode(descriptor, comparison_mode);
}


size_t LayoutDescriptor::length(const char *buffer) {

    auto ptr = buffer;

    int32_t r8d_depth = 0;


    while (1) {
        auto rax_current_ptr = ptr;
        const uint8_t current_value = *rax_current_ptr;
        ptr += 1;

        if (current_value >= uint8_t(Builder::Type::Count)) {
            continue;
        }
        const Builder::Type type = Builder::Type(current_value);
        switch (type) {
            case Builder::Type::CloseExpression: {

                return rax_current_ptr - buffer + 1;
            }
            case Builder::Type::Equatable: {

                ptr = rax_current_ptr + sizeof(const Metadata *) + sizeof(WitnessTablePointer);
                break;
            }
            case Builder::Type::Indirect: {
                ptr = rax_current_ptr + sizeof(const Metadata *);
                break;
            }
            case Builder::Type::Existential: {
                ptr = rax_current_ptr + sizeof(const Metadata *);
                break;
            }
            case Builder::Type::HeapRef:
            case Builder::Type::CaptureRef: {
                break;
            }
            case Builder::Type::Type_0x6: {
                bool condition = false;
                do {
                    condition = (*(const int8_t *)(rax_current_ptr + 1)) < 0;
                    rax_current_ptr += 1;
                } while (condition);
                r8d_depth += 1;
                ptr = rax_current_ptr + 1 + sizeof(const Metadata *);
                break;
            }
            case Builder::Type::Enum:
            case Builder::Type::Enum_0x8:
            case Builder::Type::Enum_0x9: {

                ptr = rax_current_ptr + 1 + sizeof(const Metadata *);
                break;
            }
            case Builder::Type::Type_0xA: {

                bool condition = false;
                do {
                    condition = (*(const int8_t *)(ptr)) < 0;
                    ptr += 1;
                } while (condition);
                if (r8d_depth == 0) {
                    ptr -= 1;
                    rax_current_ptr = ptr;
                    return rax_current_ptr - buffer + 1;
                }
                break;
            }
            case Builder::Type::Case_0:
            case Builder::Type::Case_1:
            case Builder::Type::Case_2:
            case Builder::Type::Case_3:
            case Builder::Type::Case_4:
            case Builder::Type::Case_5:
            case Builder::Type::Case_6:
            case Builder::Type::Case_7:
            case Builder::Type::Case_8: {
                ptr = rax_current_ptr + 1 + sizeof(const Metadata *);
                break;
            }
            case Builder::Type::CloseExpressionTwice: {
                r8d_depth -= 1;
                if (r8d_depth >= 0) {
                    break;
                }
                return rax_current_ptr - buffer + 1;
            }
        }
    }


    return 0;
}

void LayoutDescriptor::print(const char *buffer) {

    const auto _print = [&](const char *format, ...) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
    };

    _print("(layout #:length %d", length(buffer));


    char prefix_padding[1024] = {0};
    *((uint32_t *)&prefix_padding) = '  \x0A\x00';
    size_t current_prefix_length = 3;
    auto ptr = buffer;


    const uint8_t current_value = *ptr;

    if ((current_value & Builder::ActionReadMask) == Builder::ActionReadMask) {
        if ((current_value & Builder::ActionReadValueMask)) {
            const uint8_t read_bytes =
            (current_value & Builder::ActionReadValueMask) + 1;
            fputs(prefix_padding, stderr);
            _print("(read %u)", read_bytes);
        }
    } else if ((current_value & Builder::ActionSkipMask) == Builder::ActionSkipMask) {
        fputs(prefix_padding, stderr);
        const uint8_t skip_bytes =
        (current_value & Builder::ActionSkipValueMask);
        _print("(skip %u)", skip_bytes);
    } else if (current_value < uint8_t(Builder::Type::Count)) {
        const Builder::Type type = Builder::Type(current_value);

        switch (type) {
            case Builder::Type::CloseExpression: {

                fputs(")\n", stderr);
                break;
            }
            case Builder::Type::Equatable: {


                fputs(prefix_padding, stderr);
                auto metadata = reinterpret_cast<const Swift::metadata *>
                (*reinterpret_cast<const uintptr_t *>(ptr + 1));
                ptr += 1 + sizeof(Metadata *) + sizeof(WitnessTable *);
                const auto size = metadata->getType()->vw_size();
                const auto name = metadata->name(false);
                _print("(== #:size %d #:type %s", size, name.data);
                break;
            }
            case Builder::Type::Indirect: {

                fputs(prefix_padding, stderr);
                auto metadata = reinterpret_cast<const Swift::metadata *>
                (*reinterpret_cast<const uintptr_t *>(ptr + 1));
                ptr += 1 + sizeof(Metadata *) + sizeof(uintptr_t);
                const auto size = metadata->getType()->vw_size();
                const auto name = metadata->name(false);
                _print("(indirect #:size %d #:type %s", size, name.data);
                break;
            }
            case Builder::Type::Existential: {

                fputs(prefix_padding, stderr);
                auto metadata = reinterpret_cast<const Swift::metadata *>
                (*reinterpret_cast<const uintptr_t *>(ptr + 1));
                ptr += 1 + sizeof(Metadata *);
                const auto size = metadata->getType()->vw_size();
                const auto name = metadata->name(false);
                _print("(existential #:size %d #:type %s", size, name.data);
                break;
            }
            case Builder::Type::HeapRef:
            case Builder::Type::CaptureRef: {

                fputs(prefix_padding, stderr);
                _print("%s",
                       type == Builder::Type::HeapRef ? "heap-ref" : "capture-ref");
                break;
            }
            case Builder::Type::Type_0x6: {

                uint32_t r12d = 0;
                uint32_t ecx = 0;

                uint32_t eax = 0;
                do {
                    eax = *ptr;
                    uint32_t edx = eax & 0x7f;
                    edx <<= ecx;
                    ptr += 1;

                    r12d += edx;
                    ecx += 7;

                } while ((eax & 0x8) > 0x8);

                break;
            }
            case Builder::Type::Enum:
            case Builder::Type::Enum_0x8:
            case Builder::Type::Enum_0x9: {

                const auto caseValue = (int32_t)type - (int32_t)Builder::Type::Enum;
                fputs(prefix_padding, stderr);
                auto metadata = reinterpret_cast<const Swift::metadata *>
                (*reinterpret_cast<const uintptr_t *>(ptr));
                ptr += sizeof(Metadata *);
                const auto size = metadata->getType()->vw_size();
                const auto name = metadata->name(false);
                _print("(enum #:size %d #:type %s", size, name.data);

                prefix_padding[current_prefix_length + 0] = ' ';
                prefix_padding[current_prefix_length + 1] = ' ';
                prefix_padding[current_prefix_length + 2] = '\0';
                fputs(prefix_padding, stderr);

                _print("(case %d", caseValue);
                prefix_padding[current_prefix_length + 2] = ' ';
                prefix_padding[current_prefix_length + 3] = ' ';
                prefix_padding[current_prefix_length + 4] = '\0';
                current_prefix_length += 4;

                break;
            }
            case Builder::Type::Type_0xA: {

                break;
            }
            case Builder::Type::Case_0:
            case Builder::Type::Case_1:
            case Builder::Type::Case_2:
            case Builder::Type::Case_3:
            case Builder::Type::Case_4:
            case Builder::Type::Case_5:
            case Builder::Type::Case_6:
            case Builder::Type::Case_7:
            case Builder::Type::Case_8: {

                const auto caseValue = (int32_t)type -
                (int32_t)Builder::Type::Case_0;

                fputc(')', stderr);

                prefix_padding[current_prefix_length - 2] = '\0';
                fputs(prefix_padding, stderr);

                _print("(case %d", caseValue);
                prefix_padding[current_prefix_length - 2] = ' ';
                prefix_padding[current_prefix_length - 1] = ' ';
                prefix_padding[current_prefix_length] = '\0';

                break;
            }
            case Builder::Type::CloseExpressionTwice: {

                fputc(')', stderr);
                fputc(')', stderr);
                prefix_padding[current_prefix_length] = 0;
                current_prefix_length -= 4;
                break;
            }
            default:
                break;
        }
    } else {

    }


}

LayoutDescriptor::Builder::Builder(const DanceUIComparisonMode comparisonMode,
                                   const HeapMode heapMode) noexcept :
_comparisonMode(comparisonMode),
_heapMode(heapMode),
_offset(0),
_fieldsBuffer(nullptr) {
}

bool LayoutDescriptor::Builder::should_visit_fields(const Swift::metadata *metadata, bool using_equatable) {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    const DanceUIComparisonMode mode = (DanceUIComparisonModeSynchronous | get_comparison_mode());
    if (!using_equatable) {
        auto result = TypeDescriptorCache::init_shared_cache().fetch(metadata, mode, HeapMode::HeapMode_0, LayoutDescriptor::CacheSlot::FromCompareHeapObjectOrShouldVisit);
        if (result.invalidated == true) {
            return true;
        }
        const LayoutDescriptor &layout_descriptor = result.layout_descriptor;
        push_back(NestedItem(get_offset(), metadata->vw_size(), 0, &layout_descriptor));
        return false;
    }
    if (const auto witnessTable = metadata->equatable()) {
        push_back(EqualsItem(get_offset(),
                             metadata->vw_size(),
                             0,
                             metadata,
                             witnessTable));
        return false;
    }
    return true;
}

bool LayoutDescriptor::Builder::visit_element(const Swift::metadata *metadata,
                                              const TypeReferenceKind ref_kind,
                                              size_t offset,
                                              size_t remaining_size) noexcept {
    auto type = metadata->getType();
    auto valueWitnessTable = type->getValueWitnesses();
    if (remaining_size == 0) {
        return true;
    }
    OffsetRAII offsetRAII(*this, offset);
    if (ref_kind != TypeReferenceKind::DirectTypeDescriptor) {
        add_field(remaining_size);
        return true;
    }
    ComparisonModeRAII comparisonModeRAII(*this, metadata);
    if (!should_visit_fields(metadata, false)) {
        return true;
    }

    const uint16_t value = (valueWitnessTable->isPOD() ? DanceUIComparisonModePod : DanceUIComparisonModeDefault) | DanceUIComparisonModeInline;
    const DanceUIComparisonMode valueComparisonMode = DanceUIComparisonMode(value);

    const bool canUseEquatable = can_use_equatable(get_comparison_mode(), valueComparisonMode);

    if (canUseEquatable) {

        if (const auto witnessTable = metadata->equatable()) {
            push_back(EqualsItem(get_offset(),
                                 type->vw_size(),
                                 0,
                                 type,
                                 witnessTable));
            return true;
        }
    }
    const size_t beforeVisitCount = size();
    if (metadata->visit(*this)) {
        return true;
    }

    size_t afterVisitCount = size();
    while (afterVisitCount > beforeVisitCount) {
        pop_back();
        afterVisitCount -= 1;
    }
    add_field(remaining_size);
    return true;
}


bool LayoutDescriptor::Builder::visit_case(const Swift::metadata *metadata,
                                           const Swift::field_record &field_record,
                                           size_t caseIndex) noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    const auto fieldsBuffer = _fieldsBuffer;
    if (caseIndex == 0) {

        push_back(EnumItem(get_offset(),
                           metadata->getType()->vw_size(),
                           0,
                           metadata->getType()));
    }
    auto &backItem = back();
    auto &item = boost::get<Builder::EnumItem>(backItem);


    item.cases.push_back({caseIndex, data().size()});

    _fieldsBuffer = &item.cases.back().buffer;

    bool result = false;

    do {
        const char *mangledTypeName = field_record.MangledTypeName ?
        field_record.MangledTypeName.get() : nullptr;
        if (!field_record.isIndirectCase() ||
            mangledTypeName != nullptr) {
            const Metadata *type = nullptr;
            if (mangledTypeName != nullptr) {
                type = metadata->mangled_type_name_ref(mangledTypeName,
                                                       nullptr);
                if (!type) {
                    _fieldsBuffer->clear();
                    result = false;
                    break;
                }
                if (field_record.isIndirectCase()) {
                    if (type->vw_size() != 0) {
                        push_back(IndirectItem(get_offset(),
                                               type->vw_size(),
                                               0,
                                               type));
                    }
                    result = true;
                } else {
                    const Swift::metadata *metadata = reinterpret_cast<const Swift::metadata *>(type);
                    ComparisonModeRAII comparisonModeRAII(*this, metadata);
                    if (should_visit_fields(metadata, false)) {
                        if (!metadata->visit(*this)) {
                            _fieldsBuffer->clear();
                            push_back(DataItem(get_offset(), type->vw_size(), type->vw_stride()));
                        }
                    }
                    result = true;
                }
            }
        } else {
            add_field(sizeof(uintptr_t));
            result = true;
        }
    } while (0);

    if (_fieldsBuffer->size() == 0 && !result) {
        item.cases.clear();
    }

    _fieldsBuffer = fieldsBuffer;

    return result;
}


bool LayoutDescriptor::Builder::visit_existential(const Swift::existential_type_metadata *metadata) noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    const swift::ExistentialTypeMetadata *type = metadata->getType();
    const ExistentialTypeRepresentation representation = metadata->representation();

    switch (get_comparison_mode()) {
        case DanceUIComparisonModeDefault:
            return false;
        case DanceUIComparisonModePod:
            if (representation == ExistentialTypeRepresentation::Class) {
                return false;
            }
            break;
        default:
            break;
    }
    push_back(ExistentialItem(get_offset(), type->vw_size(), 0, type));
    return true;
}


bool LayoutDescriptor::Builder::visit_function(const Swift::function_type_metadata *metadata) noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    const auto function_type = metadata->getType();
    if (function_type->getConvention() != FunctionMetadataConvention::Swift) {
        return false;
    }
    if (get_comparison_mode() == DanceUIComparisonModeDefault) {
        return false;
    }
    add_field(sizeof(uintptr_t));

    push_back(HeapRefItem(get_offset() + sizeof(uintptr_t), sizeof(uintptr_t), 0, true));
    return true;
}


bool LayoutDescriptor::Builder::visit_native_object(const Swift::metadata *metadata) noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    if (_get_heap_mode() != HeapMode::HeapMode_2) {
        return false;
    }

    push_back(HeapRefItem(get_offset(), sizeof(uintptr_t), 0, true));
    return true;
}

void LayoutDescriptor::Builder::add_field(size_t size) noexcept {
    if (size == 0) {
        return;
    }
    if (_fieldsBuffer && !_fieldsBuffer->empty()) {
        if (_fieldsBuffer->back().which() == get_index_of<ElementType::types, DataItem>()) {
            auto &item = boost::get<DataItem>(_fieldsBuffer->back());
            if (item.skip) {
                item.instance_size += size;
                return;
            }
        }
    }
    push_back(DataItem(get_offset(), size, 0, false));
}

LayoutDescriptor::LayoutBufferType
LayoutDescriptor::Builder::commit(const Swift::metadata *metadata) noexcept {

    LayoutBufferType &elements = data();
    using size_type = std::remove_reference<decltype(elements)>::type::size_type;

    Emitter<LayoutBufferType> emitter;
    for (size_type idx = 0; idx < elements.size(); idx++) {
        boost::apply_visitor(emitter, std::move(elements[idx]));
    }
    return std::move(emitter.data());
}

}
