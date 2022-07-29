#pragma once

struct TextBlock
{
    std::wstring        strText;
    D2D1_RECT_F         d2dLayoutRect;
    IDWriteTextFormat*  pdwFormat;
};

//struct DamF
//{
//    std::wstring        strText;
//    D2D1_RECT_F         d2dLayoutRect;
//    IDWriteTextFormat* pdwFormat;
//    float               fLifeTime = 0.f;
//    XMFLOAT3            xmf3WorldPos;
//};

class CTextBlock;
class UILayer
{
public:
    enum TEXT_TYPE
    {
        TEXT_DAMAGE, TEXT_NPC, TEXT_RESPAWN,TEXT_END
    };
    enum TEXT_COLOR
    {
        TEXT_WHITE, TEXT_RED, TEXT_COLOR_END
    };

public:
    UILayer(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    ~UILayer();

    void UpdateLabels(const std::wstring& strUIText, int n);
    void Render(UINT nFrame);
    void ReleaseResources();
    void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);

    void Update(const float& fTimeElapsed);

    void AddDamageFont(XMFLOAT3 xmf3WorldPos, wstring strText);
    void AddPlayerDamageFont(XMFLOAT3 xmf3WorldPos, wstring strText);

    void AddTextFont(queue<wstring>& queueStr);

    static XMFLOAT3 WorldToScreen(XMFLOAT3& xmf3WorldPos);
private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(m_vWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    float m_fWidth;
    float m_fHeight;

    ID3D11DeviceContext*            m_pd3d11DeviceContext = NULL;
    ID3D11On12Device*               m_pd3d11On12Device = NULL;
    IDWriteFactory5*                 m_pd2dWriteFactory = NULL;
    ID2D1Factory3*                  m_pd2dFactory = NULL;
    ID2D1Device2*                   m_pd2dDevice = NULL;
    ID2D1DeviceContext2*            m_pd2dDeviceContext = NULL;
    ID2D1SolidColorBrush*           m_pd2dTextBrush[TEXT_COLOR_END] = { NULL };
    IDWriteTextFormat*              m_pdwTextFormat = NULL;
    IDWriteTextFormat*              m_pdwDamageFontFormat = NULL;
    IDWriteTextFormat*              m_pdwPlayerDamageFontFormat = NULL;
    IDWriteTextFormat*              m_pdwRespawnFontFormat = NULL;

    IDWriteFontCollection1*         m_pdwFontCollection1 = NULL;

    std::vector<ID3D11Resource*>    m_vWrappedRenderTargets;
    std::vector<ID2D1Bitmap1*>      m_vd2dRenderTargets;
    vector<list<CTextBlock*>>         m_vecTextBlocks;
    //std::list<CDamageTextBlock*>    m_listDamageFont;

};


class CTextBlock
{
public:
    CTextBlock();
    CTextBlock(IDWriteTextFormat* pdwFormat, D2D1_RECT_F& d2dLayoutRect, const wstring& strText = L"");
    virtual ~CTextBlock();

public:
    virtual void Update(const float& fTimeElapsed) { ; }

public:
    IDWriteTextFormat*  m_pdwFormat;
    D2D1_RECT_F         m_d2dLayoutRect;
    wstring             m_strText;
    bool                m_isDead = false;
    bool                m_isRender = true;
    UILayer::TEXT_COLOR  m_eColor;
};


class CDamageTextBlock : public CTextBlock
{
public:
    CDamageTextBlock(IDWriteTextFormat* pdwFormat, D2D1_RECT_F& d2dLayoutRect, wstring& strText, XMFLOAT3& xmf3WorldPos);
    virtual ~CDamageTextBlock();

public:
    virtual void Update(const float& fTimeElapsed) override;

public:
    XMFLOAT3            m_xmf3WorldPos;
    XMFLOAT3            m_xmf3Velocity;
    XMFLOAT3            m_xmf3Accel;
    float               m_fTime = 0.f;
    float               m_fLifeTime = 0.f;

};

class CNPCTextBlock : public CTextBlock
{
public:
    CNPCTextBlock(IDWriteTextFormat* pdwFormat, D2D1_RECT_F& d2dLayoutRect, queue<wstring>& queueStr);
    virtual ~CNPCTextBlock();

public:
    virtual void Update(const float& fTimeElapsed) override;

public:
    //wstring             m_strTotalText;
    queue<wstring>      m_queueTotalText;

    float               m_fTime = 0.f;
    int                 m_iIndex = 0;
    bool                m_isSentenceEnd = false;


};

class CRespawnTextBlock : public CTextBlock
{
public:
    CRespawnTextBlock(IDWriteTextFormat* pdwFormat, D2D1_RECT_F& d2dLayoutRect);
    virtual ~CRespawnTextBlock();

public:
    virtual void Update(const float& fTimeElapsed) override;

public:
};