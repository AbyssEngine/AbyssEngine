#ifndef ABYSS_SPRITEFONT_H
#define ABYSS_SPRITEFONT_H

#include <absl/container/btree_map.h>
#include "../common/alignment.h"
#include "../common/color.h"
#include "../common/rectangle.h"
#include "../systemio/interface.h"
#include "libabyss/formats/d2/dc6.h"
#include "libabyss/formats/d2/palette.h"
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>
#include "font.h"

namespace AbyssEngine {

class SpriteFont : public IFont {
  public:
    struct Glyph {
        uint16_t FrameIndex;
        uint8_t Width;
        uint8_t Height;
    };

    struct FramePosition {
        Rectangle Rect;
        int OffsetX;
        int OffsetY;
    };

    SpriteFont(std::string_view filePath, std::string_view paletteName, bool useGlyphHeight, eBlendMode blendMode);
    ~SpriteFont() = default;
    void GetMetrics(std::string_view text, int &width, int &height) const;
    void RenderText(int x, int y, std::string_view text, RGB colorMod, eAlignment horizontalAlignment);

  private:
    void RegenerateAtlas();
    std::unique_ptr<LibAbyss::DC6> _dc6;
    std::unique_ptr<ITexture> _atlas;
    absl::btree_map<int, Glyph> _glyphs;
    std::vector<FramePosition> _frameRects;
    const LibAbyss::Palette &_palette;
    bool _useGlyphHeight = true;
    Glyph _fallback;
};

} // namespace AbyssEngine

#endif // ABYSS_SPRITEFONT_H
