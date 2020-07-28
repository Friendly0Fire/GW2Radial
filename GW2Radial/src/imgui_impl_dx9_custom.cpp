// ImGui Win32 + DirectX9 binding
// In this binding, ImTextureID is used to store a 'LPDIRECT3DTEXTURE9' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include <imgui.h>
#include "../imgui/examples/imgui_impl_dx9.h"
#include <d912pxy.h>

// DirectX
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// Data
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9  g_pVB = NULL;
static LPDIRECT3DINDEXBUFFER9   g_pIB = NULL;
static LPDIRECT3DTEXTURE9       g_FontTexture = NULL;
static int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

//d912pxy ==============

//D3D9 API extenders =======================

#define doLocalFatal(x) MessageBoxA(0, "d912pxy imgui failed somewhere", "imgui fatal error", MB_OK);

bool g_d912pxy_present = false;

class d912pxy_renderer
{
	struct {
		DWORD VS[118] = {
			0xFFFE0300, 0x0014FFFE, 0x42415443, 0x0000001C, 0x00000023, 0xFFFE0300, 0x00000000, 0x00000000, 0x00000100, 0x0000001C,
			0x335F7376, 0x4D00305F, 0x6F726369, 0x74666F73, 0x29522820, 0x534C4820, 0x6853204C, 0x72656461, 0x6D6F4320, 0x656C6970,
			0x30312072, 0xAB00312E, 0x05000051, 0xA00F0000, 0xBF000000, 0x3F000000, 0x3F800000, 0x00000000, 0x0200001F, 0x80000000,
			0x900F0000, 0x0200001F, 0x80000005, 0x900F0001, 0x0200001F, 0x8000000A, 0x900F0002, 0x0200001F, 0x80010005, 0x900F0003,
			0x0200001F, 0x80000000, 0xE00F0000, 0x0200001F, 0x8000000A, 0xE00F0001, 0x0200001F, 0x80000005, 0xE0030002, 0x04000004,
			0x800F0000, 0x90040000, 0xA0FA0000, 0xA0BF0000, 0x02000001, 0x80060001, 0xA0FF0000, 0x03000002, 0x800F0002, 0xA0140000,
			0x90500003, 0x02000006, 0x80010002, 0x80000002, 0x03000002, 0x80010001, 0x80000002, 0x80000002, 0x03000002, 0x80030003,
			0xA0550000, 0x91E10003, 0x02000006, 0x80010002, 0x80550003, 0x02000006, 0x80010003, 0x80000003, 0x03000002, 0x80020003,
			0x80000003, 0x80000003, 0x03000005, 0x80080001, 0x80000002, 0x80550002, 0x03000009, 0xE0010000, 0x80E40000, 0x80E40001,
			0x02000006, 0x80010000, 0x80FF0002, 0x03000005, 0x80080000, 0x80000000, 0x80AA0002, 0x03000005, 0x80070000, 0xA0FA0000,
			0x90C40000, 0x02000001, 0x800D0003, 0xA0B70000, 0x03000009, 0xE0020000, 0x80E40000, 0x80E40003, 0x02000001, 0xE00C0000,
			0xA0940000, 0x02000001, 0xE00F0001, 0x90C60002, 0x02000001, 0xE0030002, 0x90E40001, 0x0000FFFF
		};
		DWORD PS[51] = {
			0xFFFF0300, 0x001FFFFE, 0x42415443, 0x0000001C, 0x0000004F, 0xFFFF0300, 0x00000001, 0x0000001C, 0x00000100, 0x00000048,
			0x00000030, 0x00000003, 0x00000001, 0x00000038, 0x00000000, 0x44325F73, 0xABABAB00, 0x000C0004, 0x00010001, 0x00000001,
			0x00000000, 0x335F7370, 0x4D00305F, 0x6F726369, 0x74666F73, 0x29522820, 0x534C4820, 0x6853204C, 0x72656461, 0x6D6F4320,
			0x656C6970, 0x30312072, 0xAB00312E, 0x0200001F, 0x8000000A, 0x900F0000, 0x0200001F, 0x80000005, 0x90030001, 0x0200001F,
			0x90000000, 0xA00F0800, 0x03000042, 0x800F0000, 0x90E40001, 0xA0E40800, 0x03000005, 0x800F0800, 0x80E40000, 0x90E40000,
			0x0000FFFF
		};
	} shaderCode;

	struct auxBuffContent
	{
		float displaySizeX;
		float displaySizeY;
		float unused0;
		float unused1;
	};

	auxBuffContent oldAuxBuff;

	float oldDisplaySize[2] = { 0,0 };	
	void* drawPSO = nullptr;
	
	DWORD textureSet[4] = { 0,0,0,0 };
	DWORD samplerSet[4] = { 0,0,0,0 };

	template<typename VStreamType, typename Element>
	class VStreamWrapper
	{
		VStreamType* vstream = nullptr;
		bool writtenThisFrame = false;
		int activeOffset = 0;
		int lastOffset = 0;
		int size = 0;
	public:
		VStreamWrapper() = default;

		~VStreamWrapper()
		{
			release();
		}

		void release()
		{
			if (vstream)
				vstream->Release();
			vstream = nullptr;
			activeOffset = 0;
			lastOffset = 0;
		}

		int getOffset()
		{
			return activeOffset * sizeof(Element);
		}

		void onFrameEnd()
		{
			writtenThisFrame = false;			
		}

		template<typename T>
		static T* createVStream(int size);

		template<typename streamType>
		void bind(streamType* stream, int stage);

		template<>
		static IDirect3DVertexBuffer9* createVStream(int size)
		{
			IDirect3DVertexBuffer9* ret;

			if (g_pd3dDevice->CreateVertexBuffer(size * sizeof(Element), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &ret, NULL) < 0)
			{
				doLocalFatal("vb not created");
				return nullptr;
			}

			return ret;
		}

		template<>
		static IDirect3DIndexBuffer9* createVStream(int size)
		{
			IDirect3DIndexBuffer9* ret;

			if (g_pd3dDevice->CreateIndexBuffer(size * sizeof(Element), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(Element) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &ret, NULL) < 0)			
			{
				doLocalFatal("ib not created");
				return nullptr;
			}

			return ret;
		}

		template<>
		void bind(IDirect3DIndexBuffer9* stream, int stage)
		{
			g_pd3dDevice->SetIndices(stream);
		}

		template<>
		void bind(IDirect3DVertexBuffer9* stream, int stage)
		{
			g_pd3dDevice->SetStreamSource(stage, stream, sizeof(Element) * lastOffset, sizeof(Element));
		}

		void bind(int stage)
		{
			bind(vstream, stage);
		}

		void lock(Element** elements, int count)
		{
			if (!writtenThisFrame)		
				activeOffset = 0;		

			if (activeOffset + count > size)
			{
				if (vstream)
					vstream->Release();

				size *= 2;
				size = count >= size ? count : size;

				vstream = createVStream<VStreamType>(size);
				activeOffset = 0;
			}

			int writeSize = sizeof(Element) * count;			
			if (vstream->Lock(getOffset(), writeSize, (void**)elements, D3DLOCK_DISCARD) < 0)
			{
				doLocalFatal("vstream lock failed");
				return;
			}
			
			lastOffset = activeOffset;
			activeOffset += count;		

			writtenThisFrame = true;
		}

		void unlock()
		{
			vstream->Unlock();
		}

	};

	VStreamWrapper<IDirect3DVertexBuffer9, ImDrawVert> vb0;
	VStreamWrapper<IDirect3DVertexBuffer9, auxBuffContent> vb1;
	VStreamWrapper<IDirect3DIndexBuffer9, ImDrawIdx> ib;

	template<D3DRENDERSTATETYPE rs>
	struct ScopedState
	{
		DWORD oldVal;

		ScopedState(DWORD value)
		{
			g_pd3dDevice->GetRenderState(rs, &oldVal);
			g_pd3dDevice->SetRenderState(rs, value);
		}

		~ScopedState()
		{
			g_pd3dDevice->SetRenderState(rs, oldVal);
		}
	};

	IDirect3DVertexShader9* vs = nullptr;
	IDirect3DPixelShader9* ps = nullptr;
	IDirect3DVertexDeclaration9* vdecl = nullptr;

public:
	d912pxy_renderer() = default;

	void compilePSO()
	{
		if (g_pd3dDevice->CreateVertexShader(shaderCode.VS, &vs) < 0)
		{
			doLocalFatal("vs failed");			
		}

		if (g_pd3dDevice->CreatePixelShader(shaderCode.PS, &ps) < 0)
		{
			doLocalFatal("ps failed");			
		}

		/*WORD    Stream;     // Stream index
		WORD    Offset;     // Offset in the stream in bytes
		BYTE    Type;       // Data type
		BYTE    Method;     // Processing method
		BYTE    Usage;      // Semantics
		BYTE    UsageIndex; // Semantic index*/

		D3DVERTEXELEMENT9 vDclElements[] = {
			{0,0,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_POSITION,0},
			{0,8,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},
			{0,16,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0},
			{1,0,D3DDECLTYPE_FLOAT2,D3DDECLMETHOD_PER_VERTEX_CONSTANT,D3DDECLUSAGE_TEXCOORD,1},
			{0xFF,0,D3DDECLTYPE_UNUSED,0,0,0}
		};

		if (g_pd3dDevice->CreateVertexDeclaration(&vDclElements[0], &vdecl) < 0)
		{
			doLocalFatal("vdecl failed");
		}

		g_pd3dDevice->SetPixelShader(ps);
		g_pd3dDevice->SetVertexShader(vs);
		g_pd3dDevice->SetVertexDeclaration(vdecl);
		g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		if (g_pd3dDevice->GetRenderState(D3DRS_D912PXY_ENQUEUE_PSO_COMPILE, (DWORD*)&drawPSO) < 0)
		{
			doLocalFatal("can't queue PSO compile");
		}

		//megai2: set and save desired sampler

		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

		samplerSet[0] = 0;
		g_pd3dDevice->GetRenderState(D3DRS_D912PXY_SAMPLER_ID, &samplerSet[0]);

	}

	void WritePrimaryGeometry(ImDrawData* draw_data)
	{
		// Copy and convert all vertices into a single contiguous buffer
		ImDrawVert* vtx_dst = nullptr;
		ImDrawIdx* idx_dst = nullptr;

		if(draw_data->TotalVtxCount > 0)
		    vb0.lock(&vtx_dst, draw_data->TotalVtxCount);
		if(draw_data->TotalIdxCount > 0)
		    ib.lock(&idx_dst, draw_data->TotalIdxCount);
			
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			if(vtx_dst != nullptr && cmd_list->VtxBuffer.Size > 0)
			    memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			if(idx_dst != nullptr && cmd_list->IdxBuffer.Size > 0)
			    memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

			if(vtx_dst != nullptr)
			    vtx_dst += cmd_list->VtxBuffer.Size;
			if(idx_dst != nullptr)
			    idx_dst += cmd_list->IdxBuffer.Size;
		}

		vb0.unlock();
		ib.unlock();
	}

	void WriteAuxBuffer()
	{
		ImGuiIO& io = ImGui::GetIO();

		float R = io.DisplaySize.x + 0.5f;
		float B = io.DisplaySize.y + 0.5f;

		if ((oldAuxBuff.displaySizeX != R) || (oldAuxBuff.displaySizeY != B))
		{
			auxBuffContent* newAuxValue;
			vb1.lock(&newAuxValue, 1);
			
			newAuxValue->displaySizeX = R;
			newAuxValue->displaySizeY = B;
			oldAuxBuff = *newAuxValue;

			vb1.unlock();					
		}
	}

	void RenderLists(ImDrawData* draw_data)
	{
		// Render command lists
		int vtx_offset = 0;
		int idx_offset = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
					g_pd3dDevice->SetScissorRect(&r);

					//megai2: this will get us texture id 
					textureSet[0] = ((LPDIRECT3DTEXTURE9)pcmd->TextureId)->GetPriority();

					g_pd3dDevice->GetRenderState(D3DRS_D912PXY_GPU_WRITE, &textureSet[0]);
					g_pd3dDevice->GetRenderState(D3DRS_D912PXY_SETUP_PSO, (DWORD*)drawPSO);

					g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, idx_offset, pcmd->ElemCount / 3);
				}
				idx_offset += pcmd->ElemCount;
			}
			vtx_offset += cmd_list->VtxBuffer.Size;
		}
	}

	void RenderDrawData(ImDrawData* draw_data)
	{
		//megai2: if PSO is not compiled yet, ignore all draws
		if (!drawPSO)
			return;

		WritePrimaryGeometry(draw_data);
		WriteAuxBuffer();		
		SetupDraw();

		{
			ScopedState<D3DRS_ALPHATESTENABLE> no_atest(false);
			ScopedState<D3DRS_SCISSORTESTENABLE> do_scissor(true);
			RenderLists(draw_data);
		}

		//megai2: update dirty flags so we transfer to dx9 mode safely
		g_pd3dDevice->SetStreamSource(1, NULL, 0, 0);		
		g_pd3dDevice->SetRenderState(D3DRS_D912PXY_DRAW, 0x101);
		g_pd3dDevice->SetRenderState(D3DRS_D912PXY_SETUP_PSO, 0);
	}

	void SetupDraw()
	{
		//megai2: mark draw start so we can see that app is issuing some not default dx9 api approach
		g_pd3dDevice->SetRenderState(D3DRS_D912PXY_DRAW, 0);

		//setup saved sampler by writing directly into gpu buffer
		g_pd3dDevice->SetRenderState(D3DRS_D912PXY_GPU_WRITE, D912PXY_ENCODE_GPU_WRITE_DSC(1, D912PXY_GPU_WRITE_OFFSET_SAMPLER));
		g_pd3dDevice->GetRenderState(D3DRS_D912PXY_GPU_WRITE, &samplerSet[0]);

		//prepare to write texture id for draws
		g_pd3dDevice->SetRenderState(D3DRS_D912PXY_GPU_WRITE, D912PXY_ENCODE_GPU_WRITE_DSC(1, D912PXY_GPU_WRITE_OFFSET_TEXBIND));

		vb0.bind(0);
		vb1.bind(1);
		ib.bind(0);

		// Setup viewport
		ImGuiIO& io = ImGui::GetIO();
		D3DVIEWPORT9 vp;
		vp.X = vp.Y = 0;
		vp.Width = (DWORD)io.DisplaySize.x;
		vp.Height = (DWORD)io.DisplaySize.y;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		g_pd3dDevice->SetViewport(&vp);
	}

	void release()
	{
		vb0.release();
		vb1.release();
		ib.release();	

		if (vs)
			vs->Release();
		if (ps)
			ps->Release();
		if (vdecl)
			vdecl->Release();

		vs = nullptr;
		ps = nullptr;
		vdecl = nullptr;

		drawPSO = nullptr;
	}

	void onFrameEnd()
	{
		vb0.onFrameEnd();
		vb1.onFrameEnd();
		ib.onFrameEnd();
	}
};

d912pxy_renderer proxyRender;

void ImGui_ImplDX9_RenderDrawData_d912pxy(ImDrawData* draw_data)
{
	proxyRender.RenderDrawData(draw_data);
}

static bool ImGui_ImplDX9_Release_d912pxy_objects()
{
	proxyRender.release();

	return true;
}

static bool ImGui_ImplDX9_Create_d912pxy_objects()
{
	proxyRender.compilePSO();
	return true;
}

//d912pxy ==============

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplDX9_RenderDrawData(ImDrawData* draw_data)
{
	if(draw_data->CmdListsCount <= 0)
		return;

    // Avoid rendering when minimized
    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
        return;

	if (g_d912pxy_present)
	{
		ImGui_ImplDX9_RenderDrawData_d912pxy(draw_data);
		return;
	}

    // Create and grow buffers if needed
    if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
        g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
        if (g_pd3dDevice->CreateVertexBuffer(g_VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL) < 0)
            return;
    }
    if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
        g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
        if (g_pd3dDevice->CreateIndexBuffer(g_IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pIB, NULL) < 0)
            return;
    }

    // Backup the DX9 state
    IDirect3DStateBlock9* d3d9_state_block = NULL;
    if (g_pd3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
        return;

    // Copy and convert all vertices into a single contiguous buffer
    CUSTOMVERTEX* vtx_dst;
    ImDrawIdx* idx_dst;
    if (g_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
        return;
    if (g_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
        return;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
        for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
        {
            vtx_dst->pos[0] = vtx_src->pos.x;
            vtx_dst->pos[1] = vtx_src->pos.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000)>>16) | ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    g_pVB->Unlock();
    g_pIB->Unlock();
    g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetIndices(g_pIB);
    g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

    // Setup viewport
    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)io.DisplaySize.x;
    vp.Height = (DWORD)io.DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    g_pd3dDevice->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing
    g_pd3dDevice->SetPixelShader(NULL);
    g_pd3dDevice->SetVertexShader(NULL);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // Setup orthographic projection matrix
    // Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        const float L = 0.5f, R = io.DisplaySize.x+0.5f, T = 0.5f, B = io.DisplaySize.y+0.5f;
        D3DMATRIX mat_identity = { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } };
        D3DMATRIX mat_projection =
        {
            2.0f/(R-L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f/(T-B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L+R)/(L-R),  (T+B)/(B-T),  0.5f,  1.0f,
        };
        g_pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        g_pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }

    // Render command lists
    int vtx_offset = 0;
    int idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
                g_pd3dDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)pcmd->TextureId);
                g_pd3dDevice->SetScissorRect(&r);
                g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, idx_offset, pcmd->ElemCount/3);
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore the DX9 state
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
}

bool ImGui_ImplDX9_Init(IDirect3DDevice9* device)
{
    g_pd3dDevice = device;
	g_d912pxy_present = device->SetRenderState(D3DRS_ENABLE_D912PXY_API_HACKS, 1) == 343434;
    return true;
}

void ImGui_ImplDX9_Shutdown()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    g_pd3dDevice = NULL;
}

static bool ImGui_ImplDX9_CreateFontsTexture()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height, bytes_per_pixel;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

	// Upload texture to graphics system
	g_FontTexture = NULL;
	if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_FontTexture, NULL) < 0)
		return false;
	D3DLOCKED_RECT tex_locked_rect;
	if (g_FontTexture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
		return false;
	for (int y = 0; y < height; y++)
		memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
	g_FontTexture->UnlockRect(0);

	// Store our identifier
	io.Fonts->TexID = (void *)g_FontTexture;

	return true;
}

bool ImGui_ImplDX9_CreateDeviceObjects()
{
    if (!g_pd3dDevice)
        return false;

	if (!ImGui_ImplDX9_CreateFontsTexture())
		return false;

	if (g_d912pxy_present)
		return ImGui_ImplDX9_Create_d912pxy_objects();			

    return true;
}

void ImGui_ImplDX9_InvalidateDeviceObjects()
{
    if (!g_pd3dDevice)
        return;
    if (g_pVB)
    {
        g_pVB->Release();
        g_pVB = NULL;
    }
    if (g_pIB)
    {
        g_pIB->Release();
        g_pIB = NULL;
    }

	// At this point note that we set ImGui::GetIO().Fonts->TexID to be == g_FontTexture, so clear both.
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(g_FontTexture == io.Fonts->TexID);
	if (g_FontTexture)
		g_FontTexture->Release();
	g_FontTexture = NULL;
	io.Fonts->TexID = NULL;

	if (g_d912pxy_present)
		ImGui_ImplDX9_Release_d912pxy_objects();
}

void ImGui_ImplDX9_NewFrame()
{
	if (!g_FontTexture)
		ImGui_ImplDX9_CreateDeviceObjects();

	proxyRender.onFrameEnd();
}