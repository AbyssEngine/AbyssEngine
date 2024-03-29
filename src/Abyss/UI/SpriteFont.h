#pragma once

#include "Abyss/Common/Logging.h"
#include "Abyss/Concepts/Drawable.h"
#include "Abyss/Concepts/FontRenderer.h"
#include "Abyss/Singletons.h"
#include "Abyss/Streams/StreamReader.h"
#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_cat.h>
#include <algorithm>
#include <cstdint>
#include <string>

namespace Abyss::UI {

struct Glyph {
    uint16_t FrameIndex;
    uint8_t Width;
    uint8_t Height;
    int OffsetX;
    int OffsetY;
};

template <Concepts::Drawable T> class SpriteFont final : public Concepts::FontRenderer {
    T _drawable;
    absl::flat_hash_map<int, Glyph> _glyphs;

    auto renderText(const std::string_view text, int &width, int &height) const -> std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> override {
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture{nullptr, SDL_DestroyTexture};
        width = 0;
        height = 0;

        for (const auto &c : text) {
            const auto &[glyphFrameIndex, glyphWidth, glyphHeight, glyphOffsetX, glyphOffsetY] = _glyphs.at(c >= _glyphs.size() ? '?' : c);
            int frameWidth, frameHeight;
            _drawable.getFrameSize(glyphFrameIndex, frameWidth, frameHeight);
            width += glyphWidth + glyphOffsetX;
            height = std::max(height, frameHeight);
        }

        const auto renderer = Singletons::getRendererProvider().getRenderer();
        texture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height));
        SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_MUL);
        const auto oldTarget = SDL_GetRenderTarget(renderer);
        SDL_SetRenderTarget(renderer, texture.get());

        int drawX = 0;
        for (const auto &c : text) {
            const auto &[glyphFrameIndex, glyphWidth, glyphHeight, glyphOffsetX, glyphOffsetY] = _glyphs.at(c >= _glyphs.size() ? '?' : c);
            int frameWidth, frameHeight;
            _drawable.getFrameSize(glyphFrameIndex, frameWidth, frameHeight);
            _drawable.draw(glyphFrameIndex, drawX, height);
            drawX += glyphWidth + glyphOffsetX;
        }

        SDL_SetRenderTarget(renderer, oldTarget);

        return std::move(texture);
    }

  public:
    explicit SpriteFont(const std::string_view path, const DataTypes::Palette &palette) : _drawable(absl::StrCat(path, ".dc6")) {
        Abyss::Common::Log::debug("Loading font {}...", path);

        _drawable.setPalette(palette);

        auto tableStream = Singletons::getFileProvider().loadFile(absl::StrCat(path, ".tbl"));
        Streams::StreamReader sr(tableStream);
        char signature[5] = {};
        sr.readBytes(signature);
        if (std::string_view(signature, 5) != "Woo!\x01")
            throw std::runtime_error(absl::StrCat("Invalid font file signature: ", path));

        tableStream.ignore(7); // skip unknown bytes

        while (!tableStream.eof()) {
            const auto code = sr.readUInt16();
            auto &[glyphFrameIndex, glyphWidth, glyphHeight, glyphOffsetX, glyphOffsetY] = _glyphs[code];

            tableStream.ignore(1); // Skip a byte for some reason

            glyphWidth = sr.readUInt8();
            glyphHeight = sr.readUInt8();

            tableStream.ignore(3); // Skip 3 unknown bytes

            glyphFrameIndex = sr.readUInt16();

            if (glyphFrameIndex == 0xFFFF) {
              // FIXME: do something about this.
              glyphFrameIndex = 1;
            }

            if (glyphFrameIndex >= _drawable.getFrameCount())
                throw std::runtime_error(absl::StrCat("Invalid font file: ", path));

            _drawable.getFrameOffset(glyphFrameIndex, glyphOffsetX, glyphOffsetY);

            tableStream.ignore(4); // More magic ignores, fun!
        }
    }
};

} // namespace Abyss::UI
