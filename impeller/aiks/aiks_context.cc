// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/aiks_context.h"

#include "impeller/aiks/picture.h"
#include "impeller/entity/contents/tessellation_cache.h"

namespace impeller {

AiksContext::AiksContext(std::shared_ptr<Context> context)
    : context_(std::move(context)) {
  if (!context_ || !context_->IsValid()) {
    return;
  }

  content_context_ = std::make_unique<ContentContext>(context_);
  if (!content_context_->IsValid()) {
    return;
  }

  is_valid_ = true;
}

AiksContext::~AiksContext() = default;

bool AiksContext::IsValid() const {
  return is_valid_;
}

std::shared_ptr<Context> AiksContext::GetContext() const {
  return context_;
}

ContentContext& AiksContext::GetContentContext() const {
  return *content_context_;
}

bool AiksContext::Render(const Picture& picture, RenderTarget& render_target) {
  if (!IsValid()) {
    return false;
  }

  if (picture.pass) {
    auto res = picture.pass->Render(*content_context_, render_target);
    // FIXME(knopp): This should be called for the last surface of the frame,
    // but there's currently no way to do this.
    content_context_->GetTessellationCache().FinishFrame();
    return res;
  }

  return true;
}

}  // namespace impeller
