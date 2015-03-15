// Filename: LayoutHelper.h
// 2005-08-01 nschan Initial revision.

#ifndef LAYOUT_HELPER_INCLUDED
#define LAYOUT_HELPER_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <bitset>
#include <vector>
#include <map>

// CLayoutInfo
// Simple data class for storing layout information and
// options for a single control (CWnd). This class allows
// you to determine quickly whether an option is set or not,
// and also to retrieve that option value quickly. Option
// values use "int" type to keep the implementation simple.
// Boolean option values can be represented by 0 or 1 values.
// Floating point values can be represented by using the
// precision field. For example, a layout option value of 1333
// together with a default precision of 3 can be interpreted
// as a floating point value of 1.333.
class CLayoutInfo
{
public:
    // Layout option types.
    enum
    {
        // Sizing constraints.
        OT_MIN_WIDTH    = 0,
        OT_MAX_WIDTH    = 1,
        OT_MIN_HEIGHT   = 2,
        OT_MAX_HEIGHT   = 3,
        OT_ASPECT_RATIO = 4,

        // Positioning constraints.
        OT_MIN_LEFT = 5,
        OT_MAX_LEFT = 6,
        OT_MIN_TOP  = 7,
        OT_MAX_TOP  = 8,

        // Constraints for anchoring to the sides
        // of the attach wnd (e.g., dialog).
        OT_LEFT_OFFSET   = 9,
        OT_TOP_OFFSET    = 10,
        OT_RIGHT_OFFSET  = 11,
        OT_BOTTOM_OFFSET = 12,

        // Options to override anchoring to a side of
        // the attach wnd. Anchor instead to a moveable
        // point within the attach wnd. These options
        // only take effect if the corresponding
        // OT_xxx_OFFSET option was chosen.
        OT_LEFT_ANCHOR   = 13,
        OT_TOP_ANCHOR    = 14,
        OT_RIGHT_ANCHOR  = 15,
        OT_BOTTOM_ANCHOR = 16,

        // Center the control based on X/Y anchor points.
        OT_CENTER_XPOS = 17,
        OT_CENTER_YPOS = 18,

        OT_OPTION_COUNT = 19
    };

    // Constructor / destructor.
    CLayoutInfo();
    ~CLayoutInfo();

    // Number of decimal places for interpreting an
    // integer option value as a floating point value.
    void  SetPrecision(int precision);
    int   GetPrecision() const;

    // Manage option values. The Add method can be
    // used to update an existing option value.
    bool  AddOption(int option, int value);
    bool  RemoveOption(int option);
    bool  HasOption(int option) const;
    bool  GetOption(int option, int& value) const;

    // Set/get the reference rect for this control.
    void  SetReferenceRect(const CRect& rect);
    const CRect& GetReferenceRect() const;

    // Clear options and reset precision and reference rect.
    void  Reset();

private:
    int   m_precision;

    std::bitset<OT_OPTION_COUNT> m_options;

    std::vector<int> m_values;

    CRect m_referenceRect;
};

// CLayoutHelper
// Helper class for control positioning and sizing.
class CLayoutHelper
{
public:
    // Layout styles (algorithms).
    enum
    {
        DEFAULT_LAYOUT  = 0,
        CENTERED_LAYOUT = 1
    };

    // Constructor / destructor.
    CLayoutHelper();
    ~CLayoutHelper();

    // Attach/detach a CWnd or CDialog. This is the window
    // containing the child controls to be repositioned/resized.
    void  AttachWnd(CWnd* pWnd);
    void  DetachWnd();

    // Select the layout style (algorithm).
    void  SetLayoutStyle(int layoutStyle);
    int   GetLayoutStyle() const;

    // Set/get the reference size of the CWnd or CDialog.
    // This is the virtual size of the client area of the
    // CWnd or CDialog to be used in all layout calculations.
    void  SetReferenceSize(int width, int height);
    const CSize& GetReferenceSize() const;

    // Child control management. The Add methods can be used
    // to add a new control or update an existing one.
    bool  AddControl(CWnd* pControl);
    bool  AddControl(CWnd* pControl, const CLayoutInfo& info);
    bool  AddChildControls();
    bool  RemoveControl(CWnd* pControl);
    bool  GetLayoutInfo(CWnd* pControl, CLayoutInfo& info) const;

    // Optional: This is a threshold size for the client
    // area of the CWnd or CDialog. Below this size, layout
    // management will be turned off (so you can turn on
    // scrolling if you like instead).
    void  SetMinimumSize(int width, int height);
    const CSize& GetMinimumSize() const;

    // Optional: Set the step size in order to have the layout
    // function invoked only at fixed size increments of the
    // dialog size. This can help to improve display performance
    // by not applying layouts on every OnSize() call. A typical
    // value for the step size might be 5 or 10 pixels.
    void  SetStepSize(int stepSize);
    int   GetStepSize() const;

    // Message handling.
    void  OnSize(UINT nType, int cx, int cy);

    // Perform layout of controls.
    void  LayoutControls();

private:
    // Layout algorithms.
    void  PerformLayout(int cx, int cy);
    void  PerformDefaultLayout(int cx, int cy);
    void  PerformCenteredLayout(int cx, int cy);

    // Helper functions.
    void  ApplyConstraintsX(const CLayoutInfo& info, int cx, double scaleX, int& x, int& width);
    void  ApplyConstraintsY(const CLayoutInfo& info, int cy, double scaleY, int& y, int& height);
    void  ApplyAspectRatio(const CLayoutInfo& info, int& width, int& height);
    void  ApplyCenteringConstraintX(const CLayoutInfo& info, double scaleX, int& x, int width);
    void  ApplyCenteringConstraintY(const CLayoutInfo& info, double scaleY, int& y, int height);

    CWnd* m_attachWnd;
    int   m_layoutStyle;
    CSize m_referenceSize;
    CSize m_minimumSize;

    int   m_stepSize;
    int   m_prevWndWidth;
    int   m_prevWndHeight;

    std::map<HWND,CLayoutInfo*>  m_controls;
};

#endif // LAYOUT_HELPER_INCLUDED

// END
