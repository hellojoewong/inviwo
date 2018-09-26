/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/basegl/processors/entryexitpointsprocessor.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

const ProcessorInfo EntryExitPoints::processorInfo_{
    "org.inviwo.EntryExitPoints",  // Class identifier
    "Entry Exit Points",           // Display name
    "Mesh Rendering",              // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
};
const ProcessorInfo EntryExitPoints::getProcessorInfo() const { return processorInfo_; }

EntryExitPoints::EntryExitPoints()
    : Processor()
    , inport_("geometry")
    , entryPort_("entry", DataVec4UInt16::get())
    , exitPort_("exit", DataVec4UInt16::get())
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), &inport_)
    , capNearClipping_("capNearClipping", "Cap near plane clipping", true)
    , trackball_(&camera_)
    , entryShaderCprTubular_("uv_pass_through.vert", "cpr_tubular_entry_points.frag")
    , exitShaderCprTubular_("uv_pass_through.vert", "cpr_tubular_exit_points.frag")
    , entryShaderCprPlanar_("uv_pass_through.vert", "cpr_planar_entry_points.frag")
    , exitShaderCprPlanar_("uv_pass_through.vert", "cpr_planar_exit_points.frag")
    , enableCprTubular_("enable_cpr_tubular", "CPR Tubular", false)
    , enableCprPlanar_("enable_cpr_planar", "CPR Planar", false)
    , quadGeometry_(nullptr)
    , quadGL_(nullptr) {
    addPort(inport_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperty(capNearClipping_);
    addProperty(camera_);
    addProperty(trackball_);
    entryPort_.addResizeEventListener(&camera_);

    addProperty(enableCprTubular_);
    addProperty(enableCprPlanar_);

    entryExitHelper_.getEntryExitShader().onReload(
        [this]() { invalidate(InvalidationLevel::InvalidResources); });
    entryExitHelper_.getNearClipShader().onReload(
        [this]() { invalidate(InvalidationLevel::InvalidResources); });

    entryShaderCprTubular_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    exitShaderCprTubular_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    entryShaderCprPlanar_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    exitShaderCprPlanar_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    quadGeometry_ =
        util::makeBuffer<vec2>({{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}});
    quadGL_ = quadGeometry_->getRepresentation<BufferGL>();
}

EntryExitPoints::~EntryExitPoints() {}

void EntryExitPoints::process() {
    if (enableCprPlanar_ && quadGL_) {
        // is this utilgl::activateAndClearTarget necessary?
        utilgl::activateAndClearTarget(*entryPort_.getEditableData().get(), ImageType::ColorOnly);

        entryShaderCprPlanar_.activate();
        quadGL_->enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        utilgl::activateAndClearTarget(*exitPort_.getEditableData().get(), ImageType::ColorOnly);
        exitShaderCprPlanar_.activate();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //utilgl::singleDrawImagePlaneRect();
        quadGL_->disable();
    } else {
        entryExitHelper_(*entryPort_.getEditableData().get(), *exitPort_.getEditableData().get(),
                         camera_.get(), *inport_.getData().get(), capNearClipping_.get());
    }
}

void EntryExitPoints::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace inviwo
