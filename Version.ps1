param([string]$ref, [string]$sha)

if ($ref.StartsWith("refs/tags/"))
{
    $tag = $ref.Substring("refs/tags/".Length)
    "#define GW2RADIAL_VER `"$tag ($sha)`"" | Out-File -FilePath ".\GW2Radial\include\github.h"
} else {
    $tag = $ref.Substring("refs/heads/".Length)
    "#define GW2RADIAL_VER `"nightly-$tag ($sha)`"" | Out-File -FilePath ".\GW2Radial\include\github.h"
}