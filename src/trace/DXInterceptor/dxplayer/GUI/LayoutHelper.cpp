// Filename: LayoutHelper.cpp
// 2005-08-01 nschan Initial revision.

#include "stdafx.h"
#include "LayoutHelper.h"

#include <cmath>

using namespace std;

static const int DEF_COMBOBOX_HEIGHT = 21;

// Helper function to check for CGroupLine instance.
static bool IsGroupLineControl(CWnd* pControl)
{
    ASSERT( pControl != NULL );

    CString strText;
    pControl->GetWindowText(strText);
    if ( strText == _T("CGroupLine") )
        return true;

    return false;
}

// Helper function to check if control is a MFC combobox.
static bool IsComboBoxControl(CWnd* pControl)
{
    ASSERT( pControl != NULL );

    if ( pControl->IsKindOf(RUNTIME_CLASS(CComboBox)) )
        return true;

    return false;
}

// Helper function to get client rect with possible
// modification by adding scrollbar width/height.
static void GetClientRectSB(CWnd* pWnd, CRect& rect)
{
    ASSERT( pWnd != NULL );

    CRect winRect;
    pWnd->GetWindowRect(&winRect);
    pWnd->ScreenToClient(&winRect);

    pWnd->GetClientRect(&rect);

    int cxSB = ::GetSystemMetrics(SM_CXVSCROLL);
    int cySB = ::GetSystemMetrics(SM_CYHSCROLL);

    if ( winRect.right >= (rect.right + cxSB) )
        rect.right += cxSB;
    if ( winRect.bottom >= (rect.bottom + cySB) )
        rect.bottom += cySB;
}

// CLayoutInfo /////////////////////////////////////////////////////////////

CLayoutInfo::CLayoutInfo()
{
    Reset();
}

CLayoutInfo::~CLayoutInfo()
{
}

void CLayoutInfo::SetPrecision(int precision)
{
    ASSERT( m_precision >= 0 );
    m_precision = precision;
}

int CLayoutInfo::GetPrecision() const
{
    return m_precision;
}

bool CLayoutInfo::AddOption(int option, int value)
{
    if ( 0 <= option && option < (int)m_values.size() )
    {
        m_options.set(option);
        m_values[option] = value;
        return true;
    }

    return false;
}

bool CLayoutInfo::RemoveOption(int option)
{
    if ( 0 <= option && option < (int)m_values.size() )
    {
        m_options.reset(option);
        m_values[option] = 0;
        return true;
    }

    return false;
}

bool CLayoutInfo::HasOption(int option) const
{
    if ( 0 <= option && option < (int)m_values.size() )
    {
        return m_options.at(option);
    }

    return false;
}

bool CLayoutInfo::GetOption(int option, int& value) const
{
    if ( !HasOption(option) )
        return false;

    value = m_values[option];

    return true;
}

void CLayoutInfo::SetReferenceRect(const CRect& rect)
{
    m_referenceRect = rect;
}

const CRect& CLayoutInfo::GetReferenceRect() const
{
    return m_referenceRect;
}

void CLayoutInfo::Reset()
{
    // Set the default precision. This is only used for options
    // that should be interpreted as floating point values.
    m_precision = 3;

    // Reset the bitset flags.
    m_options.reset();

    // Resize the vector to be the same size as the bitset.
    m_values.resize(OT_OPTION_COUNT);

    // Set all vector values to zero.
    for(int i = 0; i < (int)m_values.size(); ++i)
    {
        m_values[i] = 0;
    }

    // Initialize the reference rect.
    m_referenceRect.SetRect(0,0,0,0);
}

// CLayoutHelper /////////////////////////////////////////////////////////////

CLayoutHelper::CLayoutHelper()
{
    m_attachWnd     = NULL;
    m_layoutStyle   = CLayoutHelper::DEFAULT_LAYOUT;
    m_referenceSize = CSize(0,0);
    m_minimumSize   = CSize(0,0);

    m_stepSize      = 0;
    m_prevWndWidth  = 0;
    m_prevWndHeight = 0;
}

CLayoutHelper::~CLayoutHelper()
{
    DetachWnd();

    // Delete the CLayoutInfo instances stored in the map.
    map<HWND,CLayoutInfo*>::iterator i = m_controls.begin();
    for( ; i != m_controls.end(); ++i)
    {
        CLayoutInfo* pInfo = i->second;
        delete pInfo;
    }
    m_controls.clear();
}

void CLayoutHelper::AttachWnd(CWnd* pWnd)
{
    m_attachWnd = pWnd;
}

void CLayoutHelper::DetachWnd()
{
    m_attachWnd = NULL;
}

void CLayoutHelper::SetLayoutStyle(int layoutStyle)
{
    m_layoutStyle = layoutStyle;
}
    
int CLayoutHelper::GetLayoutStyle() const
{
    return m_layoutStyle;
}

void CLayoutHelper::SetReferenceSize(int width, int height)
{
    m_referenceSize = CSize(width, height);
}

const CSize& CLayoutHelper::GetReferenceSize() const
{
    return m_referenceSize;
}

bool CLayoutHelper::AddControl(CWnd* pControl)
{
    CLayoutInfo emptyInfo;
    bool status = AddControl(pControl, emptyInfo);

    return status;
}

bool CLayoutHelper::AddControl(CWnd* pControl, const CLayoutInfo& info)
{
    ASSERT( pControl != NULL );

    if ( m_attachWnd == NULL )
        return false;

    // The control must be created already in order to add it.
    if ( !::IsWindow(pControl->m_hWnd) )
        return false;

    // Don't add the control if it is a CGroupLine instance.
    // We don't want to add these because CGroupLine instances
    // are managed by the CGroupBox control.
    if ( IsGroupLineControl(pControl) )
        return false;

    // Check for valid reference rect from the layout info.
    CRect rect = info.GetReferenceRect();
    if ( rect.Width() <= 0 || rect.Height() <= 0 )
    {
        // The rect from the info instance is not valid.
        // We need to get the rect from the control itself.
        pControl->GetWindowRect(&rect);

        // Convert rect to client coordinates relative to attach wnd.
        m_attachWnd->ScreenToClient(&rect);

        // If this control is a combo box, we extend its
        // rect by the height of its drop down box too.
        if ( IsComboBoxControl(pControl) )
        {
            CRect rectDropDown;
            CComboBox* pBox = (CComboBox *)pControl;
            pBox->GetDroppedControlRect(&rectDropDown);
            rect.bottom += rectDropDown.Height();
        }
    }

    // Look for existing entry for this control in our map.
    map<HWND,CLayoutInfo*>::iterator result = m_controls.find(pControl->m_hWnd);
    if ( result == m_controls.end() )
    {
        // Add new entry to map.
        CLayoutInfo* pInfo = new CLayoutInfo(info);
        pInfo->SetReferenceRect(rect);
        m_controls.insert(make_pair(pControl->m_hWnd,pInfo));
    }
    else
    {
        // Update existing entry in map.
        CLayoutInfo* pInfo = result->second;
        *pInfo = info;
        pInfo->SetReferenceRect(rect);
    }

    return true;
}

bool CLayoutHelper::AddChildControls()
{
    if ( m_attachWnd == NULL )
        return false;

    // Loop through all the child windows of the attach wnd
    // and add each one with default (empty) layout info.
    CWnd* pChildWnd = m_attachWnd->GetWindow(GW_CHILD);
    while( pChildWnd != NULL && ::IsWindow(pChildWnd->m_hWnd) )
    {
        AddControl(pChildWnd);

        pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
    }

    return true;
}

bool CLayoutHelper::RemoveControl(CWnd* pControl)
{
    ASSERT( pControl != NULL );

    map<HWND,CLayoutInfo*>::iterator result = m_controls.find(pControl->m_hWnd);
    if ( result == m_controls.end() )
        return false;

    CLayoutInfo* pInfo = result->second;
    m_controls.erase(pControl->m_hWnd);
    delete pInfo;

    return true;
}

bool CLayoutHelper::GetLayoutInfo(CWnd* pControl, CLayoutInfo& info) const
{
    ASSERT( pControl != NULL );

    map<HWND,CLayoutInfo*>::const_iterator result = m_controls.find(pControl->m_hWnd);
    if ( result == m_controls.end() )
        return false;

    CLayoutInfo* pInfo = result->second;
    info = *pInfo;

    return true;
}

void CLayoutHelper::SetMinimumSize(int width, int height)
{
    m_minimumSize = CSize(width, height);
}

const CSize& CLayoutHelper::GetMinimumSize() const
{
    return m_minimumSize;
}

void CLayoutHelper::SetStepSize(int stepSize)
{
    m_stepSize = stepSize;
    m_prevWndWidth  = 0;
    m_prevWndHeight = 0;
}

int CLayoutHelper::GetStepSize() const
{
    return m_stepSize;
}

void CLayoutHelper::OnSize(UINT nType, int cx, int cy)
{
    if ( m_attachWnd == NULL || !::IsWindow(m_attachWnd->m_hWnd) )
        return;

    if ( (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED) && cx > 0 && cy > 0 )
    {
        // If step size option, we only allow layout at fixed size increments.
        CRect rect;
        GetClientRectSB(m_attachWnd, rect);
        if ( m_stepSize > 1 )
        {
            int diffWidth = abs(m_prevWndWidth - rect.Width());
            int diffHeight = abs(m_prevWndHeight - rect.Height());

            // The difference in width or height has to be larger than
            // step size, except for one special case which we allow
            // when the difference is zero.
            if ( diffWidth > m_stepSize || diffHeight > m_stepSize ||
                 (diffWidth == 0 && diffHeight == 0) )
            {
                PerformLayout(rect.Width(), rect.Height());
                m_prevWndWidth = rect.Width();
                m_prevWndHeight = rect.Height();
            }
        }
        else
        {
            PerformLayout(rect.Width(), rect.Height());
            m_prevWndWidth = rect.Width();
            m_prevWndHeight = rect.Height();
        }
    }
}

void CLayoutHelper::LayoutControls()
{
    if ( m_attachWnd == NULL || !::IsWindow(m_attachWnd->m_hWnd) )
        return;

    CRect rect;
    GetClientRectSB(m_attachWnd, rect);

    PerformLayout(rect.Width(), rect.Height());
}

void CLayoutHelper::PerformLayout(int cx, int cy)
{
    // Common error checking for all algorithms.
    if ( m_controls.size() == 0 )
        return;
    if ( m_attachWnd == NULL || !::IsWindow(m_attachWnd->m_hWnd) )
        return;
    if ( m_referenceSize.cx <= 0 || m_referenceSize.cy <= 0 )
        return;

    // Choose the layout algorithm/style to invoke.
    if ( m_layoutStyle == CLayoutHelper::CENTERED_LAYOUT )
    {
        PerformCenteredLayout(cx, cy);
    }
    else
    {
        PerformDefaultLayout(cx, cy);
    }
}

void CLayoutHelper::PerformDefaultLayout(int cx, int cy)
{
    // Compute the scale factor for resizing.
    double scaleX = cx * 1.0 / m_referenceSize.cx;
    double scaleY = cy * 1.0 / m_referenceSize.cy;

    // Check if we should apply layouts based on minimum size specification.
    bool doLayoutX = true;
    if ( m_minimumSize.cx > 0 )
        doLayoutX = (cx > m_minimumSize.cx);
    if ( !doLayoutX )
        scaleX = 1.0;

    bool doLayoutY = true;
    if ( m_minimumSize.cy > 0 )
        doLayoutY = (cy > m_minimumSize.cy);
    if ( !doLayoutY )
        scaleY = 1.0;

    // For each child control...
    map<HWND,CLayoutInfo*>::iterator i = m_controls.begin();
    for( ; i != m_controls.end(); ++i)
    {
        HWND controlHWND = i->first;
        CLayoutInfo* pInfo = i->second;

        // Control's reference rect must be valid.
        const CRect& refRect = pInfo->GetReferenceRect();
        if ( refRect.Width() <= 0 || refRect.Height() <= 0 )
            continue;

        // Begin calculating the parameters for the new control rect.
        int newX      = refRect.left;
        int newY      = refRect.top;
        int newWidth  = refRect.Width();
        int newHeight = refRect.Height();

        // Adjust positioning due to scrolling.
        newX -= m_attachWnd->GetScrollPos(SB_HORZ);
        newY -= m_attachWnd->GetScrollPos(SB_VERT);

        // X-direction reposition and scaling.
        if ( doLayoutX )
        {
            // Apply scaling to position and size.
            newX     = (int)(newX * scaleX);
            newWidth = (int)(refRect.Width() * scaleX);
        }

        // Y-direction reposition and scaling.
        if ( doLayoutY )
        {
            // Apply scaling to position and size.
            newY      = (int)(newY * scaleY);
            newHeight = (int)(refRect.Height() * scaleY);
        }

        // Apply layout constraints.
        ApplyConstraintsX(*pInfo, cx, scaleX, newX, newWidth);
        ApplyConstraintsY(*pInfo, cy, scaleY, newY, newHeight);
        ApplyAspectRatio(*pInfo, newWidth, newHeight);
        ApplyCenteringConstraintX(*pInfo, scaleX, newX, newWidth);
        ApplyCenteringConstraintY(*pInfo, scaleY, newY, newHeight);

        // Reposition and resize the control.
        UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
        ::SetWindowPos(controlHWND, 0, newX, newY, newWidth, newHeight, flags);
    }
}

void CLayoutHelper::PerformCenteredLayout(int cx, int cy)
{
    // Check if we should apply layouts based on minimum size specification.
    bool doLayoutX = true;
    if ( m_minimumSize.cx > 0 )
        doLayoutX = (cx > m_minimumSize.cx);

    bool doLayoutY = true;
    if ( m_minimumSize.cy > 0 )
        doLayoutY = (cy > m_minimumSize.cy);

    // Find the min and max extent covered by all the child controls.
    int minX = INT_MAX;
    int minY = INT_MAX;
    int maxX = 0;
    int maxY = 0;
    map<HWND,CLayoutInfo*>::iterator i = m_controls.begin();
    for( ; i != m_controls.end(); ++i)
    {
        CWnd* pControl = m_attachWnd->FromHandle(i->first);
        CLayoutInfo* pInfo = i->second;

        // Control's reference rect must be valid.
        const CRect& refRect = pInfo->GetReferenceRect();
        if ( refRect.Width() <= 0 || refRect.Height() <= 0 )
            continue;

        minX = min(minX, refRect.left);
        minY = min(minY, refRect.top);
        maxX = max(maxX, refRect.right);

        // If combobox, use its real height for extent calculation.
        if ( IsComboBoxControl(pControl) )
            maxY = max(maxY, refRect.top + DEF_COMBOBOX_HEIGHT);
        else
            maxY = max(maxY, refRect.bottom);
    }

    // Compute the width of this extent.
    int extentWidth = abs(minX - maxX);
    int extentHeight = abs(minY - maxY);

    // Compute the absolute offset to center the extent.
    int absOffsetX = max((cx - extentWidth) / 2, 0);
    int absOffsetY = max((cy - extentHeight) / 2, 0);

    // Compute the relative offset needed to move each child window control.
    int relOffsetX = absOffsetX - minX;
    int relOffsetY = absOffsetY - minY;

    // Now move each child control.
    for(i = m_controls.begin(); i != m_controls.end(); ++i)
    {
        HWND controlHWND = i->first;
        CLayoutInfo* pInfo = i->second;

        // Control's reference rect must be valid.
        const CRect& refRect = pInfo->GetReferenceRect();
        if ( refRect.Width() <= 0 || refRect.Height() <= 0 )
            continue;

        // Begin calculating the parameters for the new control rect.
        int newX      = refRect.left;
        int newY      = refRect.top;
        int newWidth  = refRect.Width();
        int newHeight = refRect.Height();

        // Adjust positioning due to scrolling.
        newX -= m_attachWnd->GetScrollPos(SB_HORZ);
        newY -= m_attachWnd->GetScrollPos(SB_VERT);

        // Apply the relative offsets for centering.
        if ( doLayoutX )
            newX += relOffsetX;
        if ( doLayoutY )
            newY += relOffsetY;

        // Reposition and resize the control.
        UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
        ::SetWindowPos(controlHWND, 0, newX, newY, newWidth, newHeight, flags);
    }
}

void CLayoutHelper::ApplyConstraintsX(const CLayoutInfo& info, int cx, double scaleX, int& x, int& width)
{
    int newX = x;
    int newWidth = width;

    int value = 0;
    int value2 = 0;

    int scrollOffset = m_attachWnd->GetScrollPos(SB_HORZ);

    // Apply sizing constraints.
    if ( info.GetOption(CLayoutInfo::OT_MIN_WIDTH, value) )
        newWidth = max(value, newWidth);
    if ( info.GetOption(CLayoutInfo::OT_MAX_WIDTH, value) )
        newWidth = min(value, newWidth);
    
    // Apply positioning constraints.
    if ( info.GetOption(CLayoutInfo::OT_MIN_LEFT, value) )
        newX = max(value - scrollOffset, newX);
    if ( info.GetOption(CLayoutInfo::OT_MAX_LEFT, value) )
        newX = min(value - scrollOffset, newX);

    // Apply anchoring constraints.
    if ( info.GetOption(CLayoutInfo::OT_LEFT_OFFSET, value) )
    {
        // Check if we anchor to the left side of attach wnd
        // or to a moveable point.
        if ( info.GetOption(CLayoutInfo::OT_LEFT_ANCHOR, value2) )
            newX = value + (int)((value2-scrollOffset) * scaleX);
        else
            newX = value - scrollOffset;
    }
    if ( info.GetOption(CLayoutInfo::OT_RIGHT_OFFSET, value) )
    {
        // Check if we anchor to the right side of attach wnd
        // or to a moveable point.
        if ( info.GetOption(CLayoutInfo::OT_RIGHT_ANCHOR, value2) )
            newWidth = max((int)((value2-scrollOffset) * scaleX) - value - newX, 0);
        else
            newWidth = max((int)((m_referenceSize.cx - scrollOffset) * scaleX) - value - newX, 0);

        // Re-check sizing constraints.
        if ( info.GetOption(CLayoutInfo::OT_MAX_WIDTH, value) )
        {
            if ( newWidth > value )
            {
                newX = newX + newWidth - value;
                newWidth = value;
            }
        }
        if ( info.GetOption(CLayoutInfo::OT_MIN_WIDTH, value) )
        {
            if ( newWidth < value )
            {
                newX = newX + newWidth - value;
                newWidth = value;
            }
        }
    }

    x = newX;
    width = newWidth;
}

void CLayoutHelper::ApplyConstraintsY(const CLayoutInfo& info, int cy, double scaleY, int& y, int& height)
{
    int newY = y;
    int newHeight = height;

    int value = 0;
    int value2 = 0;

    int scrollOffset = m_attachWnd->GetScrollPos(SB_VERT);

    // Apply sizing constraints.
    if ( info.GetOption(CLayoutInfo::OT_MIN_HEIGHT, value) )
        newHeight = max(value, newHeight);
    if ( info.GetOption(CLayoutInfo::OT_MAX_HEIGHT, value) )
        newHeight = min(value, newHeight);

    // Apply positioning constraints.
    if ( info.GetOption(CLayoutInfo::OT_MIN_TOP, value) )
        newY = max(value - scrollOffset, newY);
    if ( info.GetOption(CLayoutInfo::OT_MAX_TOP, value) )
        newY = min(value - scrollOffset, newY);
    
    // Apply anchoring constraints.
    if ( info.GetOption(CLayoutInfo::OT_TOP_OFFSET, value) )
    {
        // Check if we anchor to the top side of attach wnd
        // or to a moveable point.
        if ( info.GetOption(CLayoutInfo::OT_TOP_ANCHOR, value2) )
            newY = value + (int)((value2-scrollOffset) * scaleY);
        else
            newY = value - scrollOffset;
    }
    if ( info.GetOption(CLayoutInfo::OT_BOTTOM_OFFSET, value) )
    {
        // Check if we anchor to the bottom side of attach wnd
        // or to a moveable point.
        if ( info.GetOption(CLayoutInfo::OT_BOTTOM_ANCHOR, value2) )
            newHeight = max((int)((value2-scrollOffset) * scaleY) - value - newY, 0);
        else
            newHeight = max((int)((m_referenceSize.cy - scrollOffset) * scaleY) - value - newY, 0);

        // Re-check sizing constraints.
        if ( info.GetOption(CLayoutInfo::OT_MAX_HEIGHT, value) )
        {
            if ( newHeight > value )
            {
                newY = newY + newHeight - value;
                newHeight = value;
            }
        }
        if ( info.GetOption(CLayoutInfo::OT_MIN_HEIGHT, value) )
        {
            if ( newHeight < value )
            {
                newY = newY + newHeight - value;
                newHeight = value;
            }
        }
    }

    y = newY;
    height = newHeight;
}

void CLayoutHelper::ApplyAspectRatio(const CLayoutInfo& info, int& width, int& height)
{
    int value = 0;

    if ( !info.GetOption(CLayoutInfo::OT_ASPECT_RATIO, value) )
        return;

    int newWidth  = width;
    int newHeight = height;
    
    if ( newHeight > 0 && value > 0 )
    {
        double newRatio = pow(10.0, -1 * info.GetPrecision()) * value;

        newWidth = (int)(height * newRatio);
        newHeight = (int)(1.0 * width / newRatio);

        if ( newWidth < width )
            newHeight = height;
        else
            newWidth = width;
    }
    
    width  = newWidth;
    height = newHeight;
}

void CLayoutHelper::ApplyCenteringConstraintX(const CLayoutInfo& info, double scaleX, int& x, int width)
{
    int newX = x;

    int value = 0;

    int scrollOffset = m_attachWnd->GetScrollPos(SB_HORZ);

    if ( info.GetOption(CLayoutInfo::OT_CENTER_XPOS, value) )
    {
        int center = (int)((value - scrollOffset) * scaleX);
        newX = center - width / 2;
    }

    x = newX;
}

void CLayoutHelper::ApplyCenteringConstraintY(const CLayoutInfo& info, double scaleY, int& y, int height)
{
    int newY = y;

    int value = 0;

    int scrollOffset = m_attachWnd->GetScrollPos(SB_VERT);

    if ( info.GetOption(CLayoutInfo::OT_CENTER_YPOS, value) )
    {
        int center = (int)((value - scrollOffset) * scaleY);
        newY = center - height / 2;
    }

    y = newY;
}

// END