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

    /* Version */
    const char* freelan_get_version_string(void);

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

    typedef union {
        void* as_null;
        const char* as_string;
        int64_t as_integer;
        double as_float;
        int as_boolean;
    } FreeLANLogPayloadValue;

    struct FreeLANLogPayload {
        const char* key;
        FreeLANLogPayloadType type;
        FreeLANLogPayloadValue value;
    };

    struct FreeLANLogger;

    typedef int (*FreeLANLogFunctionCallback)(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file, unsigned int line);

    const FreeLANLogLevel FREELAN_LOG_LEVEL_TRACE = 10;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_DEBUG = 20;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_INFORMATION = 30;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_IMPORTANT = 40;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_WARNING = 50;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_ERROR = 60;
    const FreeLANLogLevel FREELAN_LOG_LEVEL_FATAL = 70;

    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_NULL = 0;
    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_STRING = 1;
    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_INTEGER = 2;
    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_FLOAT = 3;
    const FreeLANLogPayloadType FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN = 4;

    void freelan_set_log_function(FreeLANLogFunctionCallback cb);
    void freelan_set_log_level(FreeLANLogLevel level);
    FreeLANLogLevel freelan_get_log_level(void);
    int freelan_log(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, size_t payload_size, const struct FreeLANLogPayload* payload, const char* file, unsigned int line);
    struct FreeLANLogger* freelan_log_start(FreeLANLogLevel level, FreeLANTimestamp timestamp, const char* domain, const char* code, const char* file, unsigned int line);
    void freelan_log_attach(struct FreeLANLogger* logger, const char* key, FreeLANLogPayloadType type, FreeLANLogPayloadValue value);
    int freelan_log_complete(struct FreeLANLogger* logger);

    /* Types */
    struct IPv4Address;
    struct IPv6Address;
    struct Hostname;
    struct PortNumber;
    struct IPv4PrefixLength;
    struct IPv6PrefixLength;
    struct IPv4Endpoint;
    struct IPv6Endpoint;
    struct HostnameEndpoint;
    struct IPv4Route;
    struct IPv6Route;
    struct IPAddress;

    struct IPv4Address* freelan_IPv4Address_from_string(struct ErrorContext* ectx, const char* str);
    char* freelan_IPv4Address_to_string(struct ErrorContext* ectx, struct IPv4Address* inst);
    struct IPv4Address* freelan_IPv4Address_clone(struct ErrorContext* ectx, const struct IPv4Address* inst);
    void freelan_IPv4Address_free(struct IPv4Address* inst);
    int freelan_IPv4Address_less_than(const struct IPv4Address* lhs, const struct IPv4Address* rhs);
    int freelan_IPv4Address_equal(const struct IPv4Address* lhs, const struct IPv4Address* rhs);
    struct IPv6Address* freelan_IPv6Address_from_string(struct ErrorContext* ectx, const char* str);
    char* freelan_IPv6Address_to_string(struct ErrorContext* ectx, struct IPv6Address* inst);
    struct IPv6Address* freelan_IPv6Address_clone(struct ErrorContext* ectx, const struct IPv6Address* inst);
    void freelan_IPv6Address_free(struct IPv6Address* inst);
    int freelan_IPv6Address_less_than(const struct IPv6Address* lhs, const struct IPv6Address* rhs);
    int freelan_IPv6Address_equal(const struct IPv6Address* lhs, const struct IPv6Address* rhs);
    struct Hostname* freelan_Hostname_from_string(struct ErrorContext* ectx, const char* str);
    char* freelan_Hostname_to_string(struct ErrorContext* ectx, struct Hostname* inst);
    struct Hostname* freelan_Hostname_clone(struct ErrorContext* ectx, const struct Hostname* inst);
    void freelan_Hostname_free(struct Hostname* inst);
    int freelan_Hostname_less_than(const struct Hostname* lhs, const struct Hostname* rhs);
    int freelan_Hostname_equal(const struct Hostname* lhs, const struct Hostname* rhs);
    struct PortNumber* freelan_PortNumber_from_string(struct ErrorContext* ectx, const char* str);
    char* freelan_PortNumber_to_string(struct ErrorContext* ectx, struct PortNumber* inst);
    struct PortNumber* freelan_PortNumber_clone(struct ErrorContext* ectx, const struct PortNumber* inst);
    void freelan_PortNumber_free(struct PortNumber* inst);
    int freelan_PortNumber_less_than(const struct PortNumber* lhs, const struct PortNumber* rhs);
    int freelan_PortNumber_equal(const struct PortNumber* lhs, const struct PortNumber* rhs);
    struct IPv4PrefixLength* freelan_IPv4PrefixLength_from_string(struct ErrorContext* ectx, const char* str);
    char* freelan_IPv4PrefixLength_to_string(struct ErrorContext* ectx, struct IPv4PrefixLength* inst);
    struct IPv4PrefixLength* freelan_IPv4PrefixLength_clone(struct ErrorContext* ectx, const struct IPv4PrefixLength* inst);
    void freelan_IPv4PrefixLength_free(struct IPv4PrefixLength* inst);
    int freelan_IPv4PrefixLength_less_than(const struct IPv4PrefixLength* lhs, const struct IPv4PrefixLength* rhs);
    int freelan_IPv4PrefixLength_equal(const struct IPv4PrefixLength* lhs, const struct IPv4PrefixLength* rhs);
    struct IPv6PrefixLength* freelan_IPv6PrefixLength_from_string(struct ErrorContext* ectx, const char* str);
    char* freelan_IPv6PrefixLength_to_string(struct ErrorContext* ectx, struct IPv6PrefixLength* inst);
    struct IPv6PrefixLength* freelan_IPv6PrefixLength_clone(struct ErrorContext* ectx, const struct IPv6PrefixLength* inst);
    void freelan_IPv6PrefixLength_free(struct IPv6PrefixLength* inst);
    int freelan_IPv6PrefixLength_less_than(const struct IPv6PrefixLength* lhs, const struct IPv6PrefixLength* rhs);
    int freelan_IPv6PrefixLength_equal(const struct IPv6PrefixLength* lhs, const struct IPv6PrefixLength* rhs);
    struct IPv4Endpoint* freelan_IPv4Endpoint_from_string(struct ErrorContext* ectx, const char* str);
    struct IPv4Endpoint* freelan_IPv4Endpoint_from_parts(const struct IPv4Address* ip_address, const struct PortNumber* port_number);
    char* freelan_IPv4Endpoint_to_string(struct ErrorContext* ectx, const struct IPv4Endpoint* inst);
    struct IPv4Endpoint* freelan_IPv4Endpoint_clone(struct ErrorContext* ectx, const struct IPv4Endpoint* inst);
    void freelan_IPv4Endpoint_free(struct IPv4Endpoint* inst);
    int freelan_IPv4Endpoint_less_than(const struct IPv4Endpoint* lhs, const struct IPv4Endpoint* rhs);
    int freelan_IPv4Endpoint_equal(const struct IPv4Endpoint* lhs, const struct IPv4Endpoint* rhs);
    struct IPv4Address* freelan_IPv4Endpoint_get_IPv4Address(const struct IPv4Endpoint* inst);
    struct PortNumber* freelan_IPv4Endpoint_get_PortNumber(const struct IPv4Endpoint* inst);
    struct IPv6Endpoint* freelan_IPv6Endpoint_from_string(struct ErrorContext* ectx, const char* str);
    struct IPv6Endpoint* freelan_IPv6Endpoint_from_parts(const struct IPv6Address* ip_address, const struct PortNumber* port_number);
    char* freelan_IPv6Endpoint_to_string(struct ErrorContext* ectx, const struct IPv6Endpoint* inst);
    struct IPv6Endpoint* freelan_IPv6Endpoint_clone(struct ErrorContext* ectx, const struct IPv6Endpoint* inst);
    void freelan_IPv6Endpoint_free(struct IPv6Endpoint* inst);
    int freelan_IPv6Endpoint_less_than(const struct IPv6Endpoint* lhs, const struct IPv6Endpoint* rhs);
    int freelan_IPv6Endpoint_equal(const struct IPv6Endpoint* lhs, const struct IPv6Endpoint* rhs);
    struct IPv6Address* freelan_IPv6Endpoint_get_IPv6Address(const struct IPv6Endpoint* inst);
    struct PortNumber* freelan_IPv6Endpoint_get_PortNumber(const struct IPv6Endpoint* inst);
    struct HostnameEndpoint* freelan_HostnameEndpoint_from_string(struct ErrorContext* ectx, const char* str);
    struct HostnameEndpoint* freelan_HostnameEndpoint_from_parts(const struct Hostname* hostname, const struct PortNumber* port_number);
    char* freelan_HostnameEndpoint_to_string(struct ErrorContext* ectx, const struct HostnameEndpoint* inst);
    struct HostnameEndpoint* freelan_HostnameEndpoint_clone(struct ErrorContext* ectx, const struct HostnameEndpoint* inst);
    void freelan_HostnameEndpoint_free(struct HostnameEndpoint* inst);
    int freelan_HostnameEndpoint_less_than(const struct HostnameEndpoint* lhs, const struct HostnameEndpoint* rhs);
    int freelan_HostnameEndpoint_equal(const struct HostnameEndpoint* lhs, const struct HostnameEndpoint* rhs);
    struct Hostname* freelan_HostnameEndpoint_get_Hostname(const struct HostnameEndpoint* inst);
    struct PortNumber* freelan_HostnameEndpoint_get_PortNumber(const struct HostnameEndpoint* inst);
    struct IPv4Route* freelan_IPv4Route_from_string(struct ErrorContext* ectx, const char* str);
    struct IPv4Route* freelan_IPv4Route_from_parts(const struct IPv4Address* ip_address, const struct IPv4PrefixLength* prefix_length, const struct IPv4Address* gateway);
    char* freelan_IPv4Route_to_string(struct ErrorContext* ectx, const struct IPv4Route* inst);
    struct IPv4Route* freelan_IPv4Route_clone(struct ErrorContext* ectx, const struct IPv4Route* inst);
    void freelan_IPv4Route_free(struct IPv4Route* inst);
    int freelan_IPv4Route_less_than(const struct IPv4Route* lhs, const struct IPv4Route* rhs);
    int freelan_IPv4Route_equal(const struct IPv4Route* lhs, const struct IPv4Route* rhs);
    struct IPv4Address* freelan_IPv4Route_get_IPv4Address(const struct IPv4Route* inst);
    struct IPv4PrefixLength* freelan_IPv4Route_get_IPv4PrefixLength(const struct IPv4Route* inst);
    struct IPv4Address* freelan_IPv4Route_get_IPv4Address_gateway(const struct IPv4Route* inst);
    struct IPv6Route* freelan_IPv6Route_from_string(struct ErrorContext* ectx, const char* str);
    struct IPv6Route* freelan_IPv6Route_from_parts(const struct IPv6Address* ip_address, const struct IPv6PrefixLength* prefix_length, const struct IPv6Address* gateway);
    char* freelan_IPv6Route_to_string(struct ErrorContext* ectx, const struct IPv6Route* inst);
    struct IPv6Route* freelan_IPv6Route_clone(struct ErrorContext* ectx, const struct IPv6Route* inst);
    void freelan_IPv6Route_free(struct IPv6Route* inst);
    int freelan_IPv6Route_less_than(const struct IPv6Route* lhs, const struct IPv6Route* rhs);
    int freelan_IPv6Route_equal(const struct IPv6Route* lhs, const struct IPv6Route* rhs);
    struct IPv6Address* freelan_IPv6Route_get_IPv6Address(const struct IPv6Route* inst);
    struct IPv6PrefixLength* freelan_IPv6Route_get_IPv6PrefixLength(const struct IPv6Route* inst);
    struct IPv6Address* freelan_IPv6Route_get_IPv6Address_gateway(const struct IPv6Route* inst);
    struct IPAddress* freelan_IPAddress_from_string(struct ErrorContext* ectx, const char* str);
    struct IPAddress* freelan_IPAddress_from_IPv4Address(const struct IPv4Address* value);
    struct IPAddress* freelan_IPAddress_from_IPv6Address(const struct IPv6Address* value);
    const struct IPv4Address* freelan_IPAddress_as_IPv4Address(const struct IPAddress* inst);
    const struct IPv6Address* freelan_IPAddress_as_IPv6Address(const struct IPAddress* inst);
    char* freelan_IPAddress_to_string(struct ErrorContext* ectx, struct IPAddress* inst);
    struct IPAddress* freelan_IPAddress_clone(struct ErrorContext* ectx, const struct IPAddress* inst);
    void freelan_IPAddress_free(struct IPAddress* inst);
    int freelan_IPAddress_less_than(const struct IPAddress* lhs, const struct IPAddress* rhs);
    int freelan_IPAddress_equal(const struct IPAddress* lhs, const struct IPAddress* rhs);
"""

ffi.cdef(api)
ffi.set_source(
    "_pyfreelan",
    "#include <freelan.h>",
    libraries=['freelan'],
)

if __name__ == '__main__':
    ffi.compile()
