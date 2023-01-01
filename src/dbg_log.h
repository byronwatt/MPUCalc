
// stubbing out the dbg_log module.

#define LOG_STRING(...) do {} while(0)
#define MDX2_LOG4_INFO(...) do {} while(0)
#define MDX2_LOG4_ERROR(...) do {} while(0)
#define MDX2_ASSERT(x,...) assert(x)
#define MDX2_LOG_IS_WRITABLE(...) (0)
