#pragma once
#include <algorithm>
#include <cinttypes>
#include <istream>
#include <cwctype>
#include <filesystem>
#include <string>

#include <ShlObj.h>
#include <imgui.h>

#include <Main.h>
#include <Misc.h>

namespace GW2Radial
{

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr);
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

mstime TimeInMilliseconds();

int GetShaderFuncLength(const DWORD *pFunction);

// Reverse iteration wrappers for use in range-based for-loops
// ReSharper disable CppInconsistentNaming

// ReSharper disable once CppImplicitDefaultConstructorNotAvailable
template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }

std::span<byte> LoadResource(UINT resId);

// ReSharper restore CppInconsistentNaming

inline float Lerp(float a, float b, float s)
{
	if (s < 0)
		return a;
	else if (s > 1)
		return b;
	else
		return (1 - s) * a + s * b;
}

inline float SmoothStep(float x)
{
	return 3 * x * x - 2 * x * x * x;
}

inline float frand()
{
	return float(rand()) / RAND_MAX;
}

template<typename T>
struct Texture
{
	ComPtr<T> texture;
	ComPtr<ID3D11ShaderResourceView> srv;
};
using Texture1D = Texture<ID3D11Texture1D>;
using Texture2D = Texture<ID3D11Texture2D>;
using Texture3D = Texture<ID3D11Texture3D>;

struct RenderTarget : public Texture<ID3D11Texture2D>
{
	ComPtr<ID3D11RenderTargetView> rtv;

	RenderTarget& operator=(const Texture2D& tex)
	{
		texture = tex.texture;
		srv = tex.srv;

		return *this;
	}
};

struct DepthStencil : public Texture<ID3D11Texture2D>
{
	ComPtr<ID3D11DepthStencilView> rtv;
};

std::pair<ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>> CreateResourceFromResource(ID3D11Device* pDev, HMODULE hModule, unsigned uResource);

template<typename T = ID3D11Texture2D>
Texture<T> CreateTextureFromResource(ID3D11Device* pDev, HMODULE hModule, unsigned uResource)
{
	auto [res, srv] = CreateResourceFromResource(pDev, hModule, uResource);

	ComPtr<T> tex;
	res->QueryInterface(tex.GetAddressOf());
	GW2_ASSERT(tex != nullptr);

	return { tex, srv };
}

template<typename T>
auto ConvertToVector4(const T& val) {
	const float IntOffset = 0.f;
	// Fundamental types
	if constexpr (std::is_floating_point_v<T>) {
		return fVector4{ float(val), float(val), float(val), float(val) };
	} else if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
		return fVector4{ float(val) + IntOffset, float(val) + IntOffset, float(val) + IntOffset, float(val) + IntOffset };
	// Float vectors
	} else if constexpr (std::is_same_v<T, fVector2>) {
		return fVector4{ val.x, val.y, val.x, val.y };
	} else if constexpr (std::is_same_v<T, fVector3>) {
		return fVector4{ val.x, val.y, val.z, val.x };
	} else if constexpr (std::is_same_v<T, fVector4>) {
		return val;
	// Int vectors
	} else if constexpr (std::is_same_v<T, iVector2>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.x) + IntOffset, float(val.y) + IntOffset };
	} else if constexpr (std::is_same_v<T, iVector3>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.z) + IntOffset, float(val.x) + IntOffset };
	} else if constexpr (std::is_same_v<T, iVector4>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.z) + IntOffset, float(val.w) + IntOffset };
	} else {
		return fVector4{ };
	}
}

inline ImVec4 ConvertVector(const fVector4& val) {
	return { val.x, val.y, val.z, val.w };
}

inline ImVec2 ConvertVector(const fVector2& val) {
	return { val.x, val.y };
}

uint RoundUpToMultipleOf(uint numToRound, uint multiple);

template<typename Char, typename It>
It SplitString(const Char* str, const Char* delim, It out)
{
	std::basic_string<Char> s(str);
	if(s.empty())
		return out;

	size_t start = 0;
    size_t end = 0;
	while ((end = s.find(delim, start)) != std::string::npos) {
        *out = s.substr(start, end - start);
		++out;
		start = end + 1;
    }

	*out = s.substr(start);
	++out;
	return out;
}

inline uint RGBAto32(const fVector4& rgb, bool scale)
{
	float s = scale ? 255.f : 1.f;
    return D3DCOLOR_RGBA(byte(rgb.x * s), byte(rgb.y * s), byte(rgb.z * s), byte(rgb.w * s));
}

inline std::string ToLower(std::string in) {
	std::transform(in.begin(), in.end(), in.begin(), [](const char c) { return std::tolower(uint8_t(c)); });
	return in;
}
inline std::wstring ToLower(std::wstring in) {
	std::transform(in.begin(), in.end(), in.begin(), [](const wchar_t c) { return std::towlower(uint16_t(c)); });
	return in;
}

inline std::string ToUpper(std::string in) {
	std::transform(in.begin(), in.end(), in.begin(), [](const char c) { return std::toupper(uint8_t(c)); });
	return in;
}
inline std::wstring ToUpper(std::wstring in) {
	std::transform(in.begin(), in.end(), in.begin(), [](const wchar_t c) { return std::towupper(uint16_t(c)); });
	return in;
}

template<typename Vec>
float Luma(const Vec& v)
{
    return v.x * 0.2126 + v.y * 0.7152 + v.z * 0.0722;
}

constexpr uint operator "" _len(const char*, size_t len) {
    return uint(len);
}

std::filesystem::path GetGameFolder();
std::optional<std::filesystem::path> GetDocumentsFolder();
std::optional<std::filesystem::path> GetAddonFolder();

template<typename T>
T SafeToUpper(T c) {
    if constexpr(std::is_same_v<T, char>)
	    return std::toupper(uint8_t(c));
	else
		return std::towupper(uint16_t(c));
}

template<typename T>
struct ci_char_traits : std::char_traits<T> {
    static bool eq(T c1, T c2) { return SafeToUpper(c1) == SafeToUpper(c2); }
    static bool ne(T c1, T c2) { return SafeToUpper(c1) != SafeToUpper(c2); }
    static bool lt(T c1, T c2) { return SafeToUpper(c1) <  SafeToUpper(c2); }
    static int compare(const T* s1, const T* s2, size_t n) {
        while(n-- != 0) {
            if(SafeToUpper(*s1) < SafeToUpper(*s2)) return -1;
            if(SafeToUpper(*s1) > SafeToUpper(*s2)) return 1;
            ++s1; ++s2;
        }
        return 0;
    }
    static const T* find(const T* s, int n, char a) {
        while(n-- > 0 && SafeToUpper(*s) != toupper(a)) {
            ++s;
        }
        return s;
    }
};

template<typename T>
concept string_like = requires(T&& t) {
	requires std::is_pointer_v<decltype(t.data())>;
	{ t.size() } -> std::convertible_to<size_t>;
	typename T::value_type;
};

template<string_like T>
auto ToCaseInsensitive(const T& s) {
	using V = typename T::value_type;
    return std::basic_string_view<V, ci_char_traits<V>>(s.data(), s.size());
}

template <typename Str>
Str Trim(const Str &scIn) {
    using Char = typename Str::value_type;
    constexpr auto trimmable = []() constexpr {
        if constexpr (std::is_same_v<Char, char>)
            return " \t\n\r";
        else if constexpr (std::is_same_v<Char, wchar_t>)
            return L" \t\n\r";
    }
    ();

    auto start = scIn.find_first_not_of(trimmable);
    auto end = scIn.find_last_not_of(trimmable);

    return scIn.substr(start, end - start + 1);
}

template<typename T>
struct PtrComparator {
	inline bool operator()(const T* a, const T* b) const {
		return (*a) < (*b);
	}
};

std::span<const wchar_t*> GetCommandLineArgs();

const wchar_t* GetCommandLineArg(const wchar_t* name);

void DrawScreenQuad(ComPtr<ID3D11DeviceContext>& ctx);

template<std::integral T, std::integral T2>
auto RoundUp(T numToRound, T2 multiple) -> std::common_type_t<T, T2>
{
	GW2_ASSERT(multiple > 0);
	return ((numToRound + multiple - 1) / multiple) * multiple;
}


struct StateBackupD3D11
{
	UINT                        ScissorRectsCount, ViewportsCount;
	D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	ID3D11RasterizerState* RS;
	ID3D11BlendState* BlendState;
	FLOAT                       BlendFactor[4];
	UINT                        SampleMask;
	UINT                        StencilRef;
	ID3D11DepthStencilState* DepthStencilState;
	ID3D11ShaderResourceView* PSShaderResource;
	ID3D11SamplerState* PSSampler;
	ID3D11PixelShader* PS;
	ID3D11VertexShader* VS;
	ID3D11GeometryShader* GS;
	UINT                        PSInstancesCount, VSInstancesCount, GSInstancesCount;
	ID3D11ClassInstance* PSInstances[256], * VSInstances[256], * GSInstances[256];   // 256 is max according to PSSetShader documentation
	D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
	ID3D11Buffer* IndexBuffer, * VertexBuffer, * VSConstantBuffer;
	UINT                        IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
	DXGI_FORMAT                 IndexBufferFormat;
	ID3D11InputLayout* InputLayout;
	ID3D11RenderTargetView* RenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D11DepthStencilView* DepthStencil;
};

void BackupD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& old);
void RestoreD3D11State(ID3D11DeviceContext* ctx, const StateBackupD3D11& old);

}