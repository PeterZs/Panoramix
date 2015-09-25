#pragma once

#include "../core/utility.hpp"
#include "../gui/scene.hpp"

#include "pi_graph.hpp"
#include "pi_graph_annotation.hpp"

namespace pano {
    namespace experimental {



        // Print PIGraph
        template <
            class SegColorerT = core::ConstantFunctor<gui::ColorTag>,
            class LinePieceColorerT = core::ConstantFunctor<gui::ColorTag>,
            class BndPieceColorerT = core::ConstantFunctor<gui::ColorTag>
        >
        inline Image3f Print(const PIGraph & mg,
        SegColorerT && segColor = gui::Transparent,
        LinePieceColorerT && lpColor = gui::Transparent,
        BndPieceColorerT && bpColor = gui::Transparent,
        int boundaryWidth = 1, int lineWidth = 2) {
            Image3f rendered = Image3f::zeros(mg.segs.size());
            // segs
            for (auto it = rendered.begin(); it != rendered.end(); ++it) {
                int seg = mg.segs(it.pos());
                gui::Color color = segColor(seg);
                *it = Vec3f(color.bluef(), color.greenf(), color.redf());
            }
            // lines
            if (lineWidth > 0) {
                for (int lp = 0; lp < mg.linePiece2line.size(); lp++) {
                    gui::Color color = lpColor(lp);
                    if (color.isTransparent())
                        continue;
                    auto & ps = mg.linePiece2samples[lp];
                    for (int i = 1; i < ps.size(); i++) {
                        auto p1 = ToPixel(mg.view.camera.toScreen(ps[i - 1]));
                        auto p2 = ToPixel(mg.view.camera.toScreen(ps[i]));
                        if (Distance(p1, p2) >= rendered.cols / 2) {
                            continue;
                        }
                        cv::clipLine(cv::Rect(0, 0, rendered.cols, rendered.rows), p1, p2);
                        cv::line(rendered, p1, p2, (cv::Scalar)color / 255.0, lineWidth);
                    }
                }
            }
            // region boundary
            if (boundaryWidth > 0) {
                for (int bp = 0; bp < mg.bndPiece2bnd.size(); bp++) {
                    gui::Color color = bpColor(bp);
                    if (color.isTransparent())
                        continue;
                    auto & e = mg.bndPiece2dirs[bp];
                    for (int i = 1; i < e.size(); i++) {
                        auto p1 = core::ToPixel(mg.view.camera.toScreen(e[i - 1]));
                        auto p2 = core::ToPixel(mg.view.camera.toScreen(e[i]));
                        if (Distance(p1, p2) >= rendered.cols / 2) {
                            continue;
                        }
                        cv::clipLine(cv::Rect(0, 0, rendered.cols, rendered.rows), p1, p2);
                        cv::line(rendered, p1, p2, (cv::Scalar)color / 255.0, boundaryWidth);
                    }
                }
            }
            return rendered;
        }



        // Print Constraints        
        //void PrintConstriants(const PIGraph & mg);


        // VisualizeReconstruction
        void VisualizeReconstruction(const std::vector<int> & ccids, const PIGraph & mg,
            const std::function<gui::Color(int vert)> & vertColor = core::ConstantFunctor<gui::Color>(gui::White),
            const std::function<void(int vert)> & vertClick = core::ConstantFunctor<void>());


        // VisualizeLayoutAnnotation
        void VisualizeLayoutAnnotation(const PILayoutAnnotation & anno);

    }
}