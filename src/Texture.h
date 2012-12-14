#ifndef Magnum_Texture_h
#define Magnum_Texture_h
/*
    Copyright © 2010, 2011, 2012 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Magnum.

    Magnum is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Magnum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Magnum::Texture, typedef Magnum::Texture1D, Magnum::Texture2D, Magnum::Texture3D
 */

#include "AbstractTexture.h"
#include "DimensionTraits.h"

namespace Magnum {

/**
@brief %Texture

Template class for one- to three-dimensional textures. See also
AbstractTexture documentation for more information.

@section Texture-usage Usage

Common usage is to fully configure all texture parameters and then set the
data from e.g. Image. Example configuration of high quality texture with
trilinear anisotropic filtering, i.e. the best you can ask for:
@code
void* data;
Image2D image({4096, 4096}, Image2D::Components::RGBA, Image2D::ComponentType::UnsignedByte, data);

Texture2D texture;
texture.setMagnificationFilter(Texture2D::Filter::Linear)
    ->setMinificationFilter(Texture2D::Filter::Linear, Texture2D::Mipmap::Linear)
    ->setWrapping(Texture2D::Wrapping::ClampToEdge)
    ->setMaxAnisotropy(Texture2D::maxSupportedAnisotropy)
    ->setData(0, Texture2D::Format::RGBA8, &image)
    ->generateMipmap();
@endcode

@attention Don't forget to fully configure the texture before use. Note that
    default configuration (if setMinificationFilter() is not called with
    another value) is to use mipmaps, so be sure to either call setMinificationFilter(),
    explicitly set all mip levels or call generateMipmap(). If using rectangle
    texture, you must also call setWrapping(), because the initial value is
    not supported on rectangle textures. See also setMagnificationFilter() and
    setBorderColor().

The texture is bound to layer specified by shader via bind(). In shader, the
texture is used via `sampler1D`, `sampler2D` or `sampler3D` depending on
dimension count. See also AbstractShaderProgram documentation for more
information.

@section Texture-array Texture arrays

You can create texture arrays by passing @ref Texture::Target "Texture2D::Target::Texture1DArray"
or @ref Texture::Target "Texture3D::Target::Texture2DArray" to constructor.

It is possible to specify each layer separately using setSubData(), but you
have to allocate the memory for all layers first, possibly by passing properly
sized empty Image to setData(). Example: 2D texture array with 16 layers of
64x64 images:
@code
Image3D dummy({64, 64, 16}, Image3D::Components::RGBA, Image3D::ComponentType::UnsignedByte, nullptr);

Texture3D texture(Texture3D::Target::Texture2DArray);
texture.setMagnificationFilter(Texture2D::Filter::Linear)
    // ...
    ->setData(0, Texture2D::Format::RGBA8, &dummy);

for(std::size_t i = 0; i != 16; ++i) {
    void* data = ...;
    Image2D image({64, 64}, Image3D::Components::RGBA, Image3D::ComponentType::UnsignedByte, image);
    texture->setSubData(0, Vector3i::zAxis(i), image);
}

// ...
@endcode

Similar approach can be used for any other texture types (e.g. setting
Texture3D data using 2D layers, Texture2D data using one-dimensional chunks
etc.).

@section Texture-rectangle Rectangle textures

Rectangle texture is created by passing @ref Texture::Target "Texture::Target::Rectangle"
to constructor. In shader, the texture is used via sampler2DRect`. Unlike
`sampler2D`, which accepts coordinates between 0 and 1, `sampler2DRect`
accepts coordinates between 0 and `textureSizeInGivenDirection-1`. Note that
rectangle textures don't support mipmapping and repeating wrapping modes, see
@ref Texture::Filter "Filter", @ref Texture::Mipmap "Mipmap" and
generateMipmap() documentation for more information.

@requires_gl Rectangle textures are not available in OpenGL ES.
@requires_gl31 Extension @extension{ARB,texture_rectangle} (rectangle textures)

@see Texture1D, Texture2D, Texture3D, CubeMapTexture, CubeMapTextureArray
@todo @extension{AMD,sparse_texture}
 */
template<std::uint8_t dimensions> class Texture: public AbstractTexture {
    public:
        static const std::uint8_t Dimensions = dimensions; /**< @brief %Texture dimension count */

        #ifdef DOXYGEN_GENERATING_OUTPUT
        /**
         * @brief %Texture target
         *
         * Each dimension has its own unique subset of these targets.
         */
        enum class Target: GLenum {
            /**
             * One-dimensional texture
             * @requires_gl Only 2D and 3D textures are available in OpenGL
             *      ES.
             */
            Texture1D = GL_TEXTURE_1D,

            Texture2D = GL_TEXTURE_2D, /**< Two-dimensional texture */

            /**
             * Three-dimensional texture
             * @requires_gles30 %Extension @es_extension{OES,texture_3D}
             */
            Texture3D = GL_TEXTURE_3D,

            /**
             * One-dimensional texture array (i.e. two dimensions in total)
             * @requires_gl30 Extension @extension{EXT,texture_array}
             * @requires_gl Only 2D and 3D textures are available in OpenGL
             *      ES.
             */
            Texture1DArray = GL_TEXTURE_1D_ARRAY,

            /**
             * Two-dimensional texture array (i.e. three dimensions in total)
             * @requires_gl30 Extension @extension{EXT,texture_array}
             * @requires_gles30 Array textures are not available in OpenGL ES
             *      2.0.
             */
            Texture2DArray = GL_TEXTURE_2D_ARRAY,

            /**
             * Rectangle texture (i.e. two dimensions)
             * @requires_gl31 Extension @extension{ARB,texture_rectangle}
             * @requires_gl Rectangle textures are not available in OpenGL ES.
             */
            Rectangle = GL_TEXTURE_RECTANGLE
        };
        #else
        typedef typename DataHelper<Dimensions>::Target Target; /**< @brief %Texture target */
        #endif

        /**
         * @brief Constructor
         * @param target            %Texture target. If not set, default value
         *      is `Target::Texture1D`, `Target::Texture2D` or
         *      `Target::Texture3D` based on dimension count.
         *
         * Creates one OpenGL texture.
         */
        inline Texture(Target target = DataHelper<Dimensions>::target()): AbstractTexture(static_cast<GLenum>(target)) {}

        /** @brief %Texture target */
        inline constexpr Target target() const { return static_cast<Target>(_target); }

        /**
         * @brief Set wrapping
         * @param wrapping          Wrapping type for all texture dimensions
         * @return Pointer to self (for method chaining)
         *
         * Sets wrapping type for coordinates out of range (0, 1) for normal
         * textures and (0, textureSizeInGivenDirection-1) for rectangle
         * textures. If @extension{EXT,direct_state_access} is not available,
         * the texture is bound to some layer before the operation. Initial
         * value is @ref AbstractTexture::Wrapping "Wrapping::Repeat".
         * @attention For rectangle textures only some modes are supported,
         *      see @ref AbstractTexture::Wrapping "Wrapping" documentation
         *      for more information.
         * @see @fn_gl{ActiveTexture}, @fn_gl{BindTexture} and @fn_gl{TexParameter}
         *      or @fn_gl_extension{TextureParameter,EXT,direct_state_access}
         *      with @def_gl{TEXTURE_WRAP_S}, @def_gl{TEXTURE_WRAP_T},
         *      @def_gl{TEXTURE_WRAP_R}
         */
        inline Texture<Dimensions>* setWrapping(const Array<Dimensions, Wrapping>& wrapping) {
            DataHelper<Dimensions>::setWrapping(this, wrapping);
            return this;
        }

        /**
         * @brief Set texture data
         * @param mipLevel          Mip level
         * @param internalFormat    Internal texture format
         * @param image             Image, ImageWrapper, BufferImage or
         *      Trade::ImageData of the same dimension count
         * @return Pointer to self (for method chaining)
         *
         * Sets texture data from given image. The image is not deleted
         * afterwards. If @extension{EXT,direct_state_access} is not available,
         * the texture is bound to some layer before the operation.
         * @see @fn_gl{ActiveTexture}, @fn_gl{BindTexture} and @fn_gl{TexImage1D}/
         *      @fn_gl{TexImage2D}/@fn_gl{TexImage3D} or
         *      @fn_gl_extension{TextureImage1D,EXT,direct_state_access}/
         *      @fn_gl_extension{TextureImage2D,EXT,direct_state_access}/
         *      @fn_gl_extension{TextureImage3D,EXT,direct_state_access}
         */
        template<class Image> inline Texture<Dimensions>* setData(GLint mipLevel, InternalFormat internalFormat, Image* image) {
            DataHelper<Dimensions>::set(this, _target, mipLevel, internalFormat, image);
            return this;
        }

        /**
         * @brief Set texture subdata
         * @param mipLevel          Mip level
         * @param offset            Offset where to put data in the texture
         * @param image             Image, ImageWrapper, BufferImage or
         *      Trade::ImageData of the same or one less dimension count
         * @return Pointer to self (for method chaining)
         *
         * Sets texture subdata from given image. The image is not deleted
         * afterwards. The image can have either the same dimension count or
         * have one dimension less, but at least one dimension.
         *
         * If the image has one dimension less than the texture, the image is
         * taken as if it had the last dimension equal to 1. It can be used
         * for e.g. updating 3D texture with multiple 2D images or for filling
         * 1D texture array (which is two-dimensional) with 1D images.
         *
         * If @extension{EXT,direct_state_access} is not available, the
         * texture is bound to some layer before the operation.
         * @see @fn_gl{ActiveTexture}, @fn_gl{BindTexture} and @fn_gl{TexSubImage1D}/
         *      @fn_gl{TexSubImage2D}/@fn_gl{TexSubImage3D} or
         *      @fn_gl_extension{TextureSubImage1D,EXT,direct_state_access}/
         *      @fn_gl_extension{TextureSubImage2D,EXT,direct_state_access}/
         *      @fn_gl_extension{TextureSubImage3D,EXT,direct_state_access}
         */
        template<class Image> inline Texture<Dimensions>* setSubData(GLint mipLevel, const typename DimensionTraits<Dimensions, GLint>::VectorType& offset, Image* image) {
            DataHelper<Dimensions>::setSub(this, _target, mipLevel, offset, image);
            return this;
        }

        /* Overloads to remove WTF-factor from method chaining order */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        inline Texture<Dimensions>* setMinificationFilter(Filter filter, Mipmap mipmap = Mipmap::BaseLevel) {
            AbstractTexture::setMinificationFilter(filter, mipmap);
            return this;
        }
        inline Texture<Dimensions>* setMagnificationFilter(Filter filter) {
            AbstractTexture::setMagnificationFilter(filter);
            return this;
        }
        #ifndef MAGNUM_TARGET_GLES
        inline Texture<Dimensions>* setBorderColor(const Color4<>& color) {
            AbstractTexture::setBorderColor(color);
            return this;
        }
        inline Texture<Dimensions>* setMaxAnisotropy(GLfloat anisotropy) {
            AbstractTexture::setMaxAnisotropy(anisotropy);
            return this;
        }
        #endif
        inline Texture<Dimensions>* generateMipmap() {
            AbstractTexture::generateMipmap();
            return this;
        }
        #endif
};

#ifndef MAGNUM_TARGET_GLES
/**
@brief One-dimensional texture

@requires_gl Only 2D and 3D textures are available in OpenGL ES.
*/
typedef Texture<1> Texture1D;
#endif

/** @brief Two-dimensional texture */
typedef Texture<2> Texture2D;

/**
@brief Three-dimensional texture

@requires_gles30 %Extension @es_extension{OES,texture_3D}
*/
typedef Texture<3> Texture3D;

}

#endif
