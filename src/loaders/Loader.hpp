//
// Created by sebastian on 20/2/20.
//

#ifndef SDL_CRT_FILTER_LOADER_HPP
#define SDL_CRT_FILTER_LOADER_HPP
#include <ResourceRoller.hpp>
#include <SDL2/SDL.h>

class Loader: public ResourceRoller {
public:
    virtual bool GetSurface(SDL_Surface*, SDL_PixelFormat&) { return false; };
    static SDL_Surface *AllocateSurface(int w, int h);
    static SDL_Surface *AllocateSurface(int w, int h, SDL_PixelFormat &format);
    //static SDL_Surface* AllocateSurface(int w, int h);
    static SDL_Rect BiggestSurfaceClipRect(SDL_Surface* src, SDL_Surface* dst);
    static SDL_Rect SmallerBlitArea( SDL_Surface* src, SDL_Surface* dst);
    inline static void SurfacePixelsCopy( SDL_Surface* src, SDL_Surface* dst );
    static bool CompareSurface(SDL_Surface* src, SDL_Surface* dst);

    static inline Uint32 get_pixel32( SDL_Surface *surface, int x, int y);
    static inline void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel);

    static inline void blank(SDL_Surface *surface) {
        SDL_FillRect(surface, nullptr, amask);
    }

    //TODO: bit endianess
    inline static void comp(Uint32 *pixel, Uint32 *R, Uint32 *G, Uint32 *B) {
        *B = (*pixel & bmask) >> 16;
        *G = (*pixel & gmask) >> 8;
        *R = *pixel & rmask ;
    }

    //TODO: bit endianess
    inline static void toPixel(Uint32 *pixel, Uint32 *R, Uint32 *G, Uint32 *B) {
        *pixel = ((*B << 16) + (*G << 8) + *R) | amask;
    }

    inline static void toPixel(Uint32 *pixel, Uint32 *R, Uint32 *G, Uint32 *B, Uint32 *A) {
        *pixel = 0;
        *pixel = ((*A << 24) + (*B << 16) + (*G << 8) + *R);
        //if (*A == 0) * pixel = 0x01F000F0;
    }

    inline static double  fromChar(int32_t* c) { return (double) *c / 0xFF; }
    inline static double  fromChar(Uint32* c)  { return (double) *c / 0xFF; }
    inline static Uint32 toChar (double* comp) { return *comp < 1? 0xFF * *comp: 0xFF; }
    inline static double  hardSaturate(double c) {
        return c;
    }

    inline static void toLuma(double *luma, Uint32 *R, Uint32 *G, Uint32 *B) {
        *luma = 0.299 * fromChar(R) + 0.587 * fromChar(G) + 0.114 * fromChar(B);
    }

    inline static void toChroma(double *Db, double *Dr, Uint32 *R, Uint32 *G, Uint32 *B) {
        *Db = -0.450 * fromChar(R) - 0.883 * fromChar(G) + 1.333 * fromChar(B);
        *Dr = -1.333 * fromChar(R) + 1.116 * fromChar(G) + 0.217 * fromChar(B);
    }

    inline static void toRGB(const double *luma, const double *Db, const double *Dr, Uint32 *R, Uint32 *G, Uint32 *B) {
        double fR = *luma + 0.000092303716148 * *Db - 0.525912630661865 * *Dr;
        double fG = *luma - 0.129132898890509 * *Db + 0.267899328207599 * *Dr;
        double fB = *luma + 0.664679059978955 * *Db - 0.000079202543533 * *Dr;
        *R = toChar(&fR);
        *G = toChar(&fG);
        *B = toChar(&fB);
    }

    inline static void blitLineScaled(SDL_Surface *src, SDL_Surface* dst, int& line, double& scale);

    inline static void blitLine(SDL_Surface *src, SDL_Surface *dst, int& line, int& dstline) {
        SDL_Rect srcrect;
        SDL_Rect dstrect;
        SDL_GetClipRect(src, &srcrect);
        SDL_GetClipRect(src, &dstrect);
        srcrect.y = line;
        dstrect.y = dstline;
        srcrect.h = 1;
        dstrect.h = 1;
        SDL_BlitSurface(src, &srcrect, dst, &dstrect);
    }

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static const Uint32 rmask = 0xff000000;
        static const Uint32 gmask = 0x00ff0000;
        static const Uint32 bmask = 0x0000ff00;
        static const Uint32 amask = 0x000000ff;
        static const Uint32 cmask = 0xffffff00;
#else
    static const Uint32 rmask = 0x000000ff;
    static const Uint32 gmask = 0x0000ff00;
    static const Uint32 bmask = 0x00ff0000;
    static const Uint32 amask = 0xff000000;
    static const Uint32 cmask = 0x00ffffff;
#endif

};

SDL_Surface* Loader::AllocateSurface(int w, int h, SDL_PixelFormat& format) {
    SDL_Surface* ns = SDL_CreateRGBSurface(0, w, h, 32,
                                rmask, gmask, bmask, amask);
    SDL_Surface* optimized = SDL_ConvertSurface(ns, &format, 0);
    SDL_FreeSurface(ns);
    return optimized;
}

SDL_Surface *Loader::AllocateSurface(int w, int h) {
    return SDL_CreateRGBSurface(0, w, h, 32,
            rmask, gmask, bmask, amask);
}


bool Loader::CompareSurface(SDL_Surface *src, SDL_Surface *dst) {
    if (src->format->format == dst->format->format &&
        src->w == dst->w && src->h == dst->h) {
        bool error = false;
        SDL_LockSurface(src);
        SDL_LockSurface(dst);
        for(int x=0; x < src->w && !error; ++x)
            for(int y=0; y < src->h; ++y)
                if(get_pixel32(src, x, y) != get_pixel32(dst, x, y)) {
                    error = true;
                    break;
                }
        SDL_UnlockSurface(src);
        SDL_UnlockSurface(dst);
        return !error;
    }
    return false;
}

Uint32 Loader::get_pixel32(SDL_Surface *surface, int x, int y) {
    //Convert the pixels to 32 bit
    auto *pixels = (Uint32 *)surface->pixels;

    //Get the requested pixel
    return pixels[ ( y * surface->w ) + x ];
}

void Loader::put_pixel32(SDL_Surface *surface, int x, int y, Uint32 pixel) {
    //Convert the pixels to 32 bit
    auto *pixels = (Uint32 *)surface->pixels;
    //Set the pixel
    pixels[ ( y * surface->w ) + x ] = pixel;
}

SDL_Rect Loader::BiggestSurfaceClipRect(SDL_Surface *src, SDL_Surface *dst) {
    SDL_Rect srcsize;
    SDL_Rect dstsize;
    SDL_GetClipRect(src, &srcsize);
    SDL_GetClipRect(dst, &dstsize);
    //dstsize.w = Config::TARGET_WIDTH;
    //dstsize.h = Config::TARGET_HEIGHT;
    //dstsize.x = srcsize.w < dstsize.w ? abs((srcsize.w - dstsize.w )) / 2: 0;
    //dstsize.y = srcsize.h < dstsize.h ? abs((srcsize.h - dstsize.h )) / 2: 0;
    return dstsize;
}

inline void Loader::blitLineScaled(SDL_Surface *src, SDL_Surface* dst, int& line, double& scale) {
    SDL_Rect srcrect;
    SDL_Rect dstrect;
    srcrect.x = 0;
    srcrect.y = line;
    srcrect.w = Config::SCREEN_WIDTH;
    srcrect.h = 1;
    int width  = round( (double) Config::SCREEN_WIDTH * scale );
    int center = round( (double) (Config::SCREEN_WIDTH - width) / 2 );
    dstrect.x = center;
    dstrect.y = line;
    dstrect.w = width;
    dstrect.h = 1;
    SDL_BlitScaled(src, &srcrect, dst, &dstrect);
}

SDL_Rect Loader::SmallerBlitArea(SDL_Surface *src, SDL_Surface *dst) {
    SDL_Rect srcsize;
    SDL_Rect dstsize;
    SDL_Rect retsize;
    SDL_GetClipRect(src, &srcsize);
    SDL_GetClipRect(dst, &dstsize);
    retsize.x = srcsize.x < dstsize.x ? srcsize.x: dstsize.x;
    retsize.y = srcsize.y < dstsize.y ? srcsize.y: dstsize.y;

    return retsize;
}

void Loader::SurfacePixelsCopy(SDL_Surface *src, SDL_Surface *dst) {
    size_t area = src->w * src->h * sizeof(Uint32);
    memcpy( dst->pixels , src->pixels, area );
}

#endif //SDL_CRT_FILTER_LOADER_HPP