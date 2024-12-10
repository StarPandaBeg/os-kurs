#include "lua-searcher/searcher-include.h"

static const tos_global* m_os;

void tos::searcher::internal::setup(const tos_global& os)
{
	m_os = &os;
}

const tos_global& tos::searcher::internal::os()
{
	return *m_os;
}