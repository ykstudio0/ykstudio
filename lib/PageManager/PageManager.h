//-------------------------------------------------------------
// File : PageManager.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-25
// Project : SVEMS
// Version : 0.4.0
// Description : PageManager
//-------------------------------------------------------------

#pragma once

#include "DisplayPages.h"

namespace PageManager
{
    class Manager
    {
    public:
        void Begin();

        void Next();

        void Previous();

        DisplayPages::Page Current() const;

        uint8_t SubPage() const;

        void NextSubPage();
        void ResetSubPage();

    private:
        DisplayPages::Page m_currentPage =
            DisplayPages::Page::Overview;

        uint8_t m_subPage = 0U;
        
        // static uint8_t ToIndex(
        //     DisplayPages::Page page)
        // {
        //     return static_cast<uint8_t>(page);
        // }
    };
}