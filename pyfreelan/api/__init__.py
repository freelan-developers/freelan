"""
FreeLAN API.
"""

import cffi

ffi = cffi.FFI()

api = """
    /* Misc */
    void* malloc(size_t size);
    void* realloc(void* ptr, size_t size);
    void free(void* ptr);

    /* Memory */
    void* freelan_malloc(size_t size);
    void* freelan_realloc(void* ptr, size_t size);
    void freelan_free(void* ptr);
    char* freelan_strdup(const char* str);
    void freelan_register_memory_functions(void* (*malloc_func)(size_t), void* (*realloc_func)(void*, size_t), void (*free_func)(void*), char* (*strdup_func)(const char*));
    void* freelan_mark_pointer(void* ptr, const char* file, unsigned int line);
    void freelan_register_memory_debug_functions(void* (*mark_pointer_func)(void*, const char*, unsigned int));

    /* Error */
    struct ErrorContext;

    struct ErrorContext* freelan_acquire_error_context(void);
    void freelan_release_error_context(struct ErrorContext* ectx);
    void freelan_error_context_reset(struct ErrorContext* ectx);
    const char* freelan_error_context_get_error_category(const struct ErrorContext* ectx);
    int freelan_error_context_get_error_code(const struct ErrorContext* ectx);
    const char* freelan_error_context_get_error_description(const struct ErrorContext* ectx);
    const char* freelan_error_context_get_error_file(const struct ErrorContext* ectx);
    unsigned int freelan_error_context_get_error_line(const struct ErrorContext* ectx);

    /* Log */
    typedef unsigned int FreeLANLogLevel;
    typedef double FreeLANTimestamp;
    typedef unsigned int FreeLANLogPayloadType;
    struct FreeLANLog;

    struct FreeLANLogPayload {
        const char* key;
        FreeLANLogPayloadType type;
        union {
            const char* as_string;
            int64_t as_integer;
            double as_float;
            int as_boolean;
        } value;
    };

    typedef int (*FreeLANLoggingCallback)(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file, unsigned int line);

    const FreeLANLogLevel FREELAN_LOG_LEVEL_FATAL = 10;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_ERROR = 20;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_WARNING = 30;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_IMPORTANT = 40;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_INFORMATION = 50;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_DEBUG = 60;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_TRACE = 70;

    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_STRING = 1;
    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_INTEGER = 2;
    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_FLOAT = 3;
    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN = 4;

    void freelan_set_logging_callback(FreeLANLoggingCallback cb);
    void freelan_set_log_level(FreeLANLogLevel level);
    FreeLANLogLevel freelan_get_log_level(void);
    int freelan_log(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file, unsigned int line);
    struct FreeLANLog* freelan_log_start(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, const char* file, unsigned int line);
    void freelan_log_attach_string(struct FreeLANLog* log, const char* key, const char* value);
    void freelan_log_attach_integer(struct FreeLANLog* log, const char* key, uint64_t value);
    void freelan_log_attach_float(struct FreeLANLog* log, const char* key, double value);
    void freelan_log_attach_boolean(struct FreeLANLog* log, const char* key, int value);
    int freelan_log_complete(struct FreeLANLog* log);

    /* Types */
    struct IPv4Address;
    struct IPv6Address;
    struct Hostname;
    struct PortNumber;
    struct IPv4PrefixLength;
    struct IPv6PrefixLength;

    struct IPv4Address* freelan_IPv4Address_from_string(const struct ErrorContext* ectx, const char* str);
    char* freelan_IPv4Address_to_string(const struct ErrorContext* ectx, struct IPv4Address* inst);
    void freelan_IPv4Address_free(struct IPv4Address* inst);
    int freelan_IPv4Address_less_than(const struct IPv4Address* lhs, const struct IPv4Address* rhs);
    int freelan_IPv4Address_equal(const struct IPv4Address* lhs, const struct IPv4Address* rhs);
    struct IPv6Address* freelan_IPv6Address_from_string(const struct ErrorContext* ectx, const char* str);
    char* freelan_IPv6Address_to_string(const struct ErrorContext* ectx, struct IPv6Address* inst);
    void freelan_IPv6Address_free(struct IPv6Address* inst);
    int freelan_IPv6Address_less_than(const struct IPv6Address* lhs, const struct IPv6Address* rhs);
    int freelan_IPv6Address_equal(const struct IPv6Address* lhs, const struct IPv6Address* rhs);
    struct Hostname* freelan_Hostname_from_string(const struct ErrorContext* ectx, const char* str);
    char* freelan_Hostname_to_string(const struct ErrorContext* ectx, struct Hostname* inst);
    void freelan_Hostname_free(struct Hostname* inst);
    int freelan_Hostname_less_than(const struct Hostname* lhs, const struct Hostname* rhs);
    int freelan_Hostname_equal(const struct Hostname* lhs, const struct Hostname* rhs);
    struct PortNumber* freelan_PortNumber_from_string(const struct ErrorContext* ectx, const char* str);
    char* freelan_PortNumber_to_string(const struct ErrorContext* ectx, struct PortNumber* inst);
    void freelan_PortNumber_free(struct PortNumber* inst);
    int freelan_PortNumber_less_than(const struct PortNumber* lhs, const struct PortNumber* rhs);
    int freelan_PortNumber_equal(const struct PortNumber* lhs, const struct PortNumber* rhs);
    struct IPv4PrefixLength* freelan_IPv4PrefixLength_from_string(const struct ErrorContext* ectx, const char* str);
    char* freelan_IPv4PrefixLength_to_string(const struct ErrorContext* ectx, struct IPv4PrefixLength* inst);
    void freelan_IPv4PrefixLength_free(struct IPv4PrefixLength* inst);
    int freelan_IPv4PrefixLength_less_than(const struct IPv4PrefixLength* lhs, const struct IPv4PrefixLength* rhs);
    int freelan_IPv4PrefixLength_equal(const struct IPv4PrefixLength* lhs, const struct IPv4PrefixLength* rhs);
    struct IPv6PrefixLength* freelan_IPv6PrefixLength_from_string(const struct ErrorContext* ectx, const char* str);
    char* freelan_IPv6PrefixLength_to_string(const struct ErrorContext* ectx, struct IPv6PrefixLength* inst);
    void freelan_IPv6PrefixLength_free(struct IPv6PrefixLength* inst);
    int freelan_IPv6PrefixLength_less_than(const struct IPv6PrefixLength* lhs, const struct IPv6PrefixLength* rhs);
    int freelan_IPv6PrefixLength_equal(const struct IPv6PrefixLength* lhs, const struct IPv6PrefixLength* rhs);
"""

ffi.cdef(api)

native = ffi.verify(
    source=api,
    libraries=['freelan'],
    include_dirs=['./include'],
)
