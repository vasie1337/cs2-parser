#include "renderer.h"
#include <iostream>

using namespace DirectX;

namespace Renderer {

// Camera implementation
Camera::Camera()
    : m_Position(0.0f, 0.0f, -5.0f)
    , m_Rotation(0.0f, 0.0f, 0.0f)
    , m_Forward(0.0f, 0.0f, 1.0f)
    , m_Right(1.0f, 0.0f, 0.0f)
    , m_Up(0.0f, 1.0f, 0.0f)
    , m_FieldOfView(XM_PIDIV4)
    , m_AspectRatio(16.0f / 9.0f)
    , m_NearPlane(0.1f)
    , m_FarPlane(1000.0f)
{
    UpdateVectors();
}

void Camera::SetPosition(const XMFLOAT3& position)
{
    m_Position = position;
}

void Camera::SetRotation(const XMFLOAT3& rotation)
{
    m_Rotation = rotation;
    UpdateVectors();
}

void Camera::MoveForward(float distance)
{
    XMVECTOR pos = XMLoadFloat3(&m_Position);
    XMVECTOR forward = XMLoadFloat3(&m_Forward);
    
    pos = XMVectorAdd(pos, XMVectorScale(forward, distance));
    
    XMStoreFloat3(&m_Position, pos);
}

void Camera::MoveRight(float distance)
{
    XMVECTOR pos = XMLoadFloat3(&m_Position);
    XMVECTOR right = XMLoadFloat3(&m_Right);
    
    pos = XMVectorAdd(pos, XMVectorScale(right, distance));
    
    XMStoreFloat3(&m_Position, pos);
}

void Camera::MoveUp(float distance)
{
    XMVECTOR pos = XMLoadFloat3(&m_Position);
    XMVECTOR up = XMLoadFloat3(&m_Up);
    
    pos = XMVectorAdd(pos, XMVectorScale(up, distance));
    
    XMStoreFloat3(&m_Position, pos);
}

void Camera::RotateYaw(float angle)
{
    m_Rotation.y += angle;
    UpdateVectors();
}

void Camera::RotatePitch(float angle)
{
    m_Rotation.x += angle;
    
    // Clamp pitch to avoid gimbal lock
    if (m_Rotation.x > XM_PIDIV2 - 0.1f)
        m_Rotation.x = XM_PIDIV2 - 0.1f;
    if (m_Rotation.x < -XM_PIDIV2 + 0.1f)
        m_Rotation.x = -XM_PIDIV2 + 0.1f;
        
    UpdateVectors();
}

void Camera::RotateRoll(float angle)
{
    m_Rotation.z += angle;
    UpdateVectors();
}

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR pos = XMLoadFloat3(&m_Position);
    XMVECTOR lookAt = XMVectorAdd(pos, XMLoadFloat3(&m_Forward));
    XMVECTOR up = XMLoadFloat3(&m_Up);
    
    return XMMatrixLookAtLH(pos, lookAt, up);
}

XMMATRIX Camera::GetProjectionMatrix() const
{
    return XMMatrixPerspectiveFovLH(m_FieldOfView, m_AspectRatio, m_NearPlane, m_FarPlane);
}

void Camera::UpdateVectors()
{
    // Create rotation matrix based on yaw, pitch, roll
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    
    // Base vectors
    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    
    // Transform vectors
    forward = XMVector3TransformNormal(forward, rotationMatrix);
    right = XMVector3TransformNormal(right, rotationMatrix);
    up = XMVector3TransformNormal(up, rotationMatrix);
    
    // Store transformed vectors
    XMStoreFloat3(&m_Forward, forward);
    XMStoreFloat3(&m_Right, right);
    XMStoreFloat3(&m_Up, up);
}

// Renderer implementation
Renderer::Renderer()
    : m_Device(nullptr)
    , m_DeviceContext(nullptr)
    , m_SwapChain(nullptr)
    , m_RenderTargetView(nullptr)
    , m_DepthStencilView(nullptr)
    , m_DepthStencilBuffer(nullptr)
    , m_VertexBuffer(nullptr)
    , m_InputLayout(nullptr)
    , m_VertexShader(nullptr)
    , m_PixelShader(nullptr)
    , m_CameraBuffer(nullptr)
    , m_WorldBuffer(nullptr)
    , m_VertexCount(0)
    , m_Width(0)
    , m_Height(0)
{
}

Renderer::~Renderer()
{
    Shutdown();
}

bool Renderer::Initialize(HWND hWnd, int width, int height)
{
    m_Width = width;
    m_Height = height;
    
    // Set aspect ratio for camera
    m_Camera.SetPosition(XMFLOAT3(0.0f, 0.0f, -300.0f));
    
    if (!InitializeDirectX(hWnd, width, height))
        return false;
        
    if (!CreateShaders())
        return false;
        
    if (!CreateConstantBuffers())
        return false;
        
    return true;
}

void Renderer::Shutdown()
{
    if (m_WorldBuffer) m_WorldBuffer->Release();
    if (m_CameraBuffer) m_CameraBuffer->Release();
    if (m_VertexBuffer) m_VertexBuffer->Release();
    if (m_InputLayout) m_InputLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
    if (m_DepthStencilView) m_DepthStencilView->Release();
    if (m_DepthStencilBuffer) m_DepthStencilBuffer->Release();
    if (m_RenderTargetView) m_RenderTargetView->Release();
    if (m_SwapChain) m_SwapChain->Release();
    if (m_DeviceContext) m_DeviceContext->Release();
    if (m_Device) m_Device->Release();
    
    m_WorldBuffer = nullptr;
    m_CameraBuffer = nullptr;
    m_VertexBuffer = nullptr;
    m_InputLayout = nullptr;
    m_VertexShader = nullptr;
    m_PixelShader = nullptr;
    m_DepthStencilView = nullptr;
    m_DepthStencilBuffer = nullptr;
    m_RenderTargetView = nullptr;
    m_SwapChain = nullptr;
    m_DeviceContext = nullptr;
    m_Device = nullptr;
}

bool Renderer::LoadTriangles(const std::vector<cs2::Triangle>& triangles)
{
    // Convert triangles to vertices
    m_Vertices.clear();
    
    for (const auto& triangle : triangles)
    {
        // Random color per triangle for better visualization
        float r = (float)rand() / RAND_MAX;
        float g = (float)rand() / RAND_MAX;
        float b = (float)rand() / RAND_MAX;
        XMFLOAT4 color(r, g, b, 1.0f);
        
        // Add three vertices for the triangle
        m_Vertices.push_back({ XMFLOAT3(triangle.a.x, triangle.a.y, triangle.a.z), color });
        m_Vertices.push_back({ XMFLOAT3(triangle.b.x, triangle.b.y, triangle.b.z), color });
        m_Vertices.push_back({ XMFLOAT3(triangle.c.x, triangle.c.y, triangle.c.z), color });
    }
    
    m_VertexCount = (int)m_Vertices.size();
    
    // Create vertex buffer
    return CreateVertexBuffer();
}

void Renderer::Render()
{
    // Clear the back buffer and depth buffer
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Black
    m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, clearColor);
    m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    // Update constant buffers
    UpdateConstantBuffers();
    
    // Set the vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_DeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    
    // Set primitive topology
    m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // Set shaders and constant buffers
    m_DeviceContext->VSSetShader(m_VertexShader, nullptr, 0);
    m_DeviceContext->PSSetShader(m_PixelShader, nullptr, 0);
    m_DeviceContext->VSSetConstantBuffers(0, 1, &m_CameraBuffer);
    m_DeviceContext->VSSetConstantBuffers(1, 1, &m_WorldBuffer);
    
    // Draw the vertices
    m_DeviceContext->Draw(m_VertexCount, 0);
    
    // Present the back buffer to the screen
    m_SwapChain->Present(1, 0);
}

bool Renderer::InitializeDirectX(HWND hWnd, int width, int height)
{
    // Create device and swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        nullptr, 0, D3D11_SDK_VERSION, 
        &swapChainDesc, &m_SwapChain, &m_Device, &featureLevel, &m_DeviceContext);
        
    if (FAILED(hr))
    {
        std::cerr << "Failed to create device and swap chain" << std::endl;
        return false;
    }
    
    // Create render target view
    ID3D11Texture2D* backBuffer = nullptr;
    hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get back buffer" << std::endl;
        return false;
    }
    
    hr = m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
    backBuffer->Release();
    if (FAILED(hr))
    {
        std::cerr << "Failed to create render target view" << std::endl;
        return false;
    }
    
    // Create depth stencil buffer and view
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = width;
    depthBufferDesc.Height = height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    
    hr = m_Device->CreateTexture2D(&depthBufferDesc, nullptr, &m_DepthStencilBuffer);
    if (FAILED(hr))
    {
        std::cerr << "Failed to create depth stencil buffer" << std::endl;
        return false;
    }
    
    hr = m_Device->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, &m_DepthStencilView);
    if (FAILED(hr))
    {
        std::cerr << "Failed to create depth stencil view" << std::endl;
        return false;
    }
    
    // Set render targets
    m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);
    
    // Set viewport
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    
    m_DeviceContext->RSSetViewports(1, &viewport);
    
    return true;
}

bool Renderer::CreateShaders()
{
    // Vertex shader code
    const char* vsCode = R"(
        cbuffer CameraBuffer : register(b0)
        {
            matrix View;
            matrix Projection;
        };
        
        cbuffer WorldBuffer : register(b1)
        {
            matrix World;
        };
        
        struct VS_INPUT
        {
            float3 Position : POSITION;
            float4 Color : COLOR;
        };
        
        struct PS_INPUT
        {
            float4 Position : SV_POSITION;
            float4 Color : COLOR;
        };
        
        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output;
            
            // Transform the position
            float4 worldPosition = mul(float4(input.Position, 1.0f), World);
            float4 viewPosition = mul(worldPosition, View);
            output.Position = mul(viewPosition, Projection);
            
            // Pass the color to the pixel shader
            output.Color = input.Color;
            
            return output;
        }
    )";
    
    // Pixel shader code
    const char* psCode = R"(
        struct PS_INPUT
        {
            float4 Position : SV_POSITION;
            float4 Color : COLOR;
        };
        
        float4 main(PS_INPUT input) : SV_TARGET
        {
            return input.Color;
        }
    )";
    
    // Compile vertex shader
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompile(vsCode, strlen(vsCode), "VS", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::cerr << "Vertex shader compilation failed: " << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        return false;
    }
    
    // Create vertex shader
    hr = m_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
    if (FAILED(hr))
    {
        vsBlob->Release();
        std::cerr << "Failed to create vertex shader" << std::endl;
        return false;
    }
    
    // Define input layout
    D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    // Create input layout
    hr = m_Device->CreateInputLayout(inputDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);
    vsBlob->Release();
    if (FAILED(hr))
    {
        std::cerr << "Failed to create input layout" << std::endl;
        return false;
    }
    
    // Set input layout
    m_DeviceContext->IASetInputLayout(m_InputLayout);
    
    // Compile pixel shader
    ID3DBlob* psBlob = nullptr;
    hr = D3DCompile(psCode, strlen(psCode), "PS", nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::cerr << "Pixel shader compilation failed: " << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        return false;
    }
    
    // Create pixel shader
    hr = m_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);
    psBlob->Release();
    if (FAILED(hr))
    {
        std::cerr << "Failed to create pixel shader" << std::endl;
        return false;
    }
    
    return true;
}

bool Renderer::CreateVertexBuffer()
{
    // Create vertex buffer
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(Vertex) * m_VertexCount;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = m_Vertices.data();
    
    // Release previous vertex buffer if it exists
    if (m_VertexBuffer)
    {
        m_VertexBuffer->Release();
        m_VertexBuffer = nullptr;
    }
    
    HRESULT hr = m_Device->CreateBuffer(&bufferDesc, &initData, &m_VertexBuffer);
    if (FAILED(hr))
    {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return false;
    }
    
    return true;
}

bool Renderer::CreateConstantBuffers()
{
    // Create camera buffer
    D3D11_BUFFER_DESC cameraBufferDesc = {};
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(CameraBuffer);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    HRESULT hr = m_Device->CreateBuffer(&cameraBufferDesc, nullptr, &m_CameraBuffer);
    if (FAILED(hr))
    {
        std::cerr << "Failed to create camera buffer" << std::endl;
        return false;
    }
    
    // Create world buffer
    D3D11_BUFFER_DESC worldBufferDesc = {};
    worldBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    worldBufferDesc.ByteWidth = sizeof(WorldBuffer);
    worldBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    worldBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    hr = m_Device->CreateBuffer(&worldBufferDesc, nullptr, &m_WorldBuffer);
    if (FAILED(hr))
    {
        std::cerr << "Failed to create world buffer" << std::endl;
        return false;
    }
    
    return true;
}

void Renderer::UpdateConstantBuffers()
{
    // Update camera buffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_DeviceContext->Map(m_CameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        CameraBuffer* cameraData = (CameraBuffer*)mappedResource.pData;
        cameraData->View = XMMatrixTranspose(m_Camera.GetViewMatrix());
        cameraData->Projection = XMMatrixTranspose(m_Camera.GetProjectionMatrix());
        m_DeviceContext->Unmap(m_CameraBuffer, 0);
    }
    
    // Update world buffer
    hr = m_DeviceContext->Map(m_WorldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        WorldBuffer* worldData = (WorldBuffer*)mappedResource.pData;
        worldData->World = XMMatrixTranspose(XMMatrixIdentity());
        m_DeviceContext->Unmap(m_WorldBuffer, 0);
    }
}

// Application implementation
Application::Application()
    : m_hWnd(nullptr)
    , m_Running(false)
    , m_LastMouseX(0)
    , m_LastMouseY(0)
    , m_MouseDown(false)
{
    memset(m_Keys, 0, sizeof(m_Keys));
}

Application::~Application()
{
    Shutdown();
}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    // Initialize window
    if (!InitializeWindow(hInstance, nCmdShow))
        return false;
    
    // Initialize renderer
    if (!m_Renderer.Initialize(m_hWnd, 1280, 720))
        return false;
    
    m_Running = true;
    return true;
}

void Application::Run()
{
    MSG msg = {};
    
    // Main loop
    while (m_Running)
    {
        // Process Windows messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT)
            {
                m_Running = false;
                break;
            }
        }
        
        if (!m_Running)
            break;
        
        // Process input and update
        ProcessInput();
        
        // Render
        m_Renderer.Render();
    }
}

void Application::Shutdown()
{
    m_Renderer.Shutdown();
    
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

bool Application::LoadTriangles(const std::vector<cs2::Triangle>& triangles)
{
    return m_Renderer.LoadTriangles(triangles);
}

bool Application::InitializeWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"CS2ViewerWindowClass";
    
    if (!RegisterClassEx(&wc))
    {
        std::cerr << "Failed to register window class" << std::endl;
        return false;
    }
    
    // Create window
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    
    m_hWnd = CreateWindow(
        L"CS2ViewerWindowClass", L"CS2 Viewer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, this);
        
    if (!m_hWnd)
    {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    
    return true;
}

void Application::ProcessInput()
{
    // Camera movement speed
    float moveSpeed = 5.0f;  // Units per frame
    float rotateSpeed = 0.01f;  // Radians per pixel
    
    Camera& camera = m_Renderer.GetCamera();
    
    // Move camera based on WASD keys
    if (m_Keys['W'])
        camera.MoveForward(moveSpeed);
    if (m_Keys['S'])
        camera.MoveForward(-moveSpeed);
    if (m_Keys['A'])
        camera.MoveRight(-moveSpeed);
    if (m_Keys['D'])
        camera.MoveRight(moveSpeed);
    
    // Move camera up/down with E/Q
    if (m_Keys['E'])
        camera.MoveUp(moveSpeed);
    if (m_Keys['Q'])
        camera.MoveUp(-moveSpeed);
    
    // Increase/decrease speed with Shift/Ctrl
    if (m_Keys[VK_SHIFT])
        moveSpeed *= 2.0f;
    if (m_Keys[VK_CONTROL])
        moveSpeed *= 0.5f;
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Get app pointer from window data
    Application* app = nullptr;
    if (message == WM_CREATE)
    {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        app = (Application*)cs->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)app);
    }
    else
    {
        app = (Application*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }
    
    // Handle messages
    switch (message)
    {
        case WM_KEYDOWN:
            if (app && wParam < 256)
                app->m_Keys[wParam] = true;
            if (wParam == VK_ESCAPE)
                PostQuitMessage(0);
            break;
            
        case WM_KEYUP:
            if (app && wParam < 256)
                app->m_Keys[wParam] = false;
            break;
            
        case WM_LBUTTONDOWN:
            if (app)
            {
                app->m_MouseDown = true;
                app->m_LastMouseX = GET_X_LPARAM(lParam);
                app->m_LastMouseY = GET_Y_LPARAM(lParam);
                SetCapture(hWnd);
            }
            break;
            
        case WM_LBUTTONUP:
            if (app)
            {
                app->m_MouseDown = false;
                ReleaseCapture();
            }
            break;
            
        case WM_MOUSEMOVE:
            if (app && app->m_MouseDown)
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                int dx = x - app->m_LastMouseX;
                int dy = y - app->m_LastMouseY;
                
                Camera& camera = app->m_Renderer.GetCamera();
                camera.RotateYaw(dx * 0.01f);
                camera.RotatePitch(dy * 0.01f);
                
                app->m_LastMouseX = x;
                app->m_LastMouseY = y;
            }
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    
    return 0;
}

} // namespace Renderer 