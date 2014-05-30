#include "basic_types.hpp"

namespace panoramix {
    namespace vis {

        namespace {
            inline Color rgb(int R, int G, int B) {
                return Color(B, G, R);
            }
            inline Color rgba(int R, int G, int B, int A) {
                return Color(B, G, R, A);
            }
        }

        Color ColorFromTag(ColorTag t) {
            switch (t){
            case ColorTag::Transparent: return rgba(0, 0, 0, 0);

            case ColorTag::White: return rgb(255, 255, 255);
            case ColorTag::Black: return rgb(0, 0, 0);

            case ColorTag::DimGray: return rgb(105, 105, 105);
            case ColorTag::Gray: return rgb(128, 128, 128);
            case ColorTag::DarkGray: return rgb(169, 169, 169);
            case ColorTag::Silver: return rgb(192, 192, 192);
            case ColorTag::LightGray: return rgb(211, 211, 211);

            case ColorTag::Red: return rgb(255, 0, 0);
            case ColorTag::Green: return rgb(0, 255, 0);
            case ColorTag::Blue: return rgb(0, 0, 255);

            case ColorTag::Yellow: return rgb(255, 255, 0);
            case ColorTag::Magenta: return rgb(255, 0, 255);
            case ColorTag::Cyan: return rgb(0, 255, 255);
            case ColorTag::Orange: return rgb(255, 165, 0);
            default:
                return Color(255, 255, 255);
            }
        }

        const std::vector<Color> & PredefinedColorTable(ColorTableDescriptor descriptor) {
            static const std::vector<Color> allColorTable = {
                ColorFromTag(ColorTag::White),
                ColorFromTag(ColorTag::Gray),
                ColorFromTag(ColorTag::Red),
                ColorFromTag(ColorTag::Green),
                ColorFromTag(ColorTag::Blue),
                ColorFromTag(ColorTag::Yellow),
                ColorFromTag(ColorTag::Magenta),
                ColorFromTag(ColorTag::Cyan),
                ColorFromTag(ColorTag::Orange)
            };
            static const std::vector<Color> WRGBColorTable = {
                ColorFromTag(ColorTag::White),
                ColorFromTag(ColorTag::Red),
                ColorFromTag(ColorTag::Green),
                ColorFromTag(ColorTag::Blue),
            };
            static const std::vector<Color> RGBColorTable = {
                ColorFromTag(ColorTag::Red),
                ColorFromTag(ColorTag::Green),
                ColorFromTag(ColorTag::Blue),
            };
            
            switch (descriptor){
            case ColorTableDescriptor::WRGB: return WRGBColorTable;
            case ColorTableDescriptor::RGB: return RGBColorTable;
            default: return allColorTable;
            }
        }

    }
}