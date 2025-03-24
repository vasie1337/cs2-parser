#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <string>
#include <Windows.h>
#include <windowsx.h>
#include "../core/cs2/parser.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace Renderer {

    struct Vertex {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT4 Color;
    };

    struct CameraBuffer {
        DirectX::XMMATRIX View;
        DirectX::XMMATRIX Projection;
    };

    struct WorldBuffer {
        DirectX::XMMATRIX World;
    };

    class Camera {
    public:
        Camera();
        
        void SetPosition(const DirectX::XMFLOAT3& position);
        void SetRotation(const DirectX::XMFLOAT3& rotation);
        
        void MoveForward(float distance);
        void MoveRight(float distance);
        void MoveUp(float distance);
        
        void RotateYaw(float angle);
        void RotatePitch(float angle);
        void RotateRoll(float angle);

        DirectX::XMMATRIX GetViewMatrix() const;
        DirectX::XMMATRIX GetProjectionMatrix() const;
        
        DirectX::XMFLOAT3 GetPosition() const { return m_Position; }
        DirectX::XMFLOAT3 GetRotation() const { return m_Rotation; }

    private:
        void UpdateVectors();

        DirectX::XMFLOAT3 m_Position;
        DirectX::XMFLOAT3 m_Rotation;
        
        DirectX::XMFLOAT3 m_Forward;
        DirectX::XMFLOAT3 m_Right;
        DirectX::XMFLOAT3 m_Up;
        
        float m_FieldOfView;
        float m_AspectRatio;
        float m_NearPlane;
        float m_FarPlane;
    };

    class Renderer {
    public:
        Renderer();
        ~Renderer();

        bool Initialize(HWND hWnd, int width, int height);
        void Shutdown();
        
        bool LoadTriangles(const std::vector<cs2::Triangle>& triangles);
        void Render();
        
        Camera& GetCamera() { return m_Camera; }

    private:
        bool InitializeDirectX(HWND hWnd, int width, int height);
        bool CreateShaders();
        bool CreateVertexBuffer();
        bool CreateConstantBuffers();
        bool CreateRasterizerState();
        void UpdateConstantBuffers();

        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_DeviceContext;
        IDXGISwapChain* m_SwapChain;
        ID3D11RenderTargetView* m_RenderTargetView;
        ID3D11DepthStencilView* m_DepthStencilView;
        ID3D11Texture2D* m_DepthStencilBuffer;
        
        ID3D11Buffer* m_VertexBuffer;
        ID3D11InputLayout* m_InputLayout;
        ID3D11VertexShader* m_VertexShader;
        ID3D11PixelShader* m_PixelShader;
        
        ID3D11Buffer* m_CameraBuffer;
        ID3D11Buffer* m_WorldBuffer;
        
        ID3D11RasterizerState* m_WireframeRasterizerState;
        
        std::vector<Vertex> m_Vertices;
        int m_VertexCount;
        
        Camera m_Camera;
        int m_Width;
        int m_Height;
    };

    class Application {
    public:
        Application();
        ~Application();

        bool Initialize(HINSTANCE hInstance, int nCmdShow);
        void Run();
        void Shutdown();
        
        bool LoadTriangles(const std::vector<cs2::Triangle>& triangles);

    private:
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        bool InitializeWindow(HINSTANCE hInstance, int nCmdShow);
        void ProcessInput();

        HWND m_hWnd;
        Renderer m_Renderer;
        bool m_Running;
        
        bool m_Keys[256];
        int m_LastMouseX;
        int m_LastMouseY;
        bool m_MouseDown;
    };
} 