#ifndef RATIOCANVAS_HH
#define RATIOCANVAS_HH
#include "TCanvas.h"
#include "TPad.h"
#include <memory>
#include <string>

class TH1;

#if 0
/*
 *  ____       _   _        ____
 * |  _ \ __ _| |_(_) ___  / ___|__ _ _ ____   ____ _ ___
 * | |_) / _` | __| |/ _ \| |   / _` | '_ \ \ / / _` / __|
 * |  _ < (_| | |_| | (_) | |__| (_| | | | \ V / (_| \__ \
 * |_| \_\__,_|\__|_|\___/ \____\__,_|_| |_|\_/ \__,_|___/
 *
 *
 */
#endif
namespace fn
{
    class RatioCanvas
    {
        public:
            RatioCanvas ( TH1& bg, TH1& dat, TH1& rat);
            void Write( std::string name );

        private:
            TCanvas result_canvas_;
            std::unique_ptr<TPad> pad_upper_;
            std::unique_ptr<TPad> pad_lower_;
    };
}
#endif

