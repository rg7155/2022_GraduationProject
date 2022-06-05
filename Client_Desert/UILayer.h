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

class CDamageTextBlock;
class UILayer
{
public:
    UILayer(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    ~UILayer();

    void UpdateLabels(const std::wstring& strUIText, int n);
    void Render(UINT nFrame);
    void ReleaseResources();
    void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);

    void Update(const float& fTimeElapsed);

    void AddDamageFont(XMFLOAT3 xmf3WorldPos, wstring strText);
    static XMFLOAT3 WorldToScreen(XMFLOAT3& xmf3WorldPos);

private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(m_vWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    void UpdateDamageFont(const float& fTimeElapsed);

    float m_fWidth;
    float m_fHeight;

    ID3D11DeviceContext*            m_pd3d11DeviceContext = NULL;
    ID3D11On12Device*               m_pd3d11On12Device = NULL;
    IDWriteFactory*                 m_pd2dWriteFactory = NULL;
    ID2D1Factory3*                  m_pd2dFactory = NULL;
    ID2D1Device2*                   m_pd2dDevice = NULL;
    ID2D1DeviceContext2*            m_pd2dDeviceContext = NULL;
    ID2D1SolidColorBrush*           m_pd2dTextBrush = NULL;
    IDWriteTextFormat*              m_pdwTextFormat = NULL;

    std::vector<ID3D11Resource*>    m_vWrappedRenderTargets;
    std::vector<ID2D1Bitmap1*>      m_vd2dRenderTargets;
    std::vector<TextBlock>          m_vTextBlocks;
    std::list<CDamageTextBlock*>    m_listDamageFont;

};


class CTextBlock
{
public:
    CTextBlock();
    CTextBlock(IDWriteTextFormat* pdwFormat, D2D1_RECT_F& d2dLayoutRect, wstring& strText);
    virtual ~CTextBlock();

public:
    virtual void Update(const float& fTimeElapsed) { ; }

public:
    IDWriteTextFormat*  m_pdwFormat;
    D2D1_RECT_F         m_d2dLayoutRect;
    wstring             m_strText;
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
    float               m_fLifeTime = 0.f;
    float               m_fTime = 0.f;
};