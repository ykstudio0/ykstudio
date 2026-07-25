//-------------------------------------------------------------
// File : PageManager.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-25
// Project : SVEMS
// Version : 0.4.0
// Description : PageManager
//-------------------------------------------------------------

#include "PageManager.h"

namespace PageManager
{
    void Manager::Begin()
    {
        m_currentPage =
            DisplayPages::Page::Overview;
    }

    DisplayPages::Page Manager::Current() const
    {
        return m_currentPage;
    }

    void Manager::Next()
    {
        uint8_t page =
            static_cast<uint8_t>(m_currentPage);

        page++;

        if(page >= static_cast<uint8_t>(DisplayPages::Page::Count))
        {
            page = 0;
        }

        // uint8_t page =
        //     ToIndex(m_currentPage);
        m_currentPage =
            static_cast<DisplayPages::Page>(page);
    }

    void Manager::Previous()
    {
        int page =
            static_cast<uint8_t>(m_currentPage);

        page--;

        if(page < 0)
        {
            page = 
                static_cast<int>(DisplayPages::Page::Count) - 1;
        }

        // uint8_t page =
        //     ToIndex(m_currentPage);
        m_currentPage =
            static_cast<DisplayPages::Page>(page);
    }
}