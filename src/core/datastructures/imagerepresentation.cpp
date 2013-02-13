#include <inviwo/core/datastructures/imagerepresentation.h>

namespace inviwo {

    ImageRepresentation::ImageRepresentation()
        : DataRepresentation()
    {}

    ImageRepresentation::ImageRepresentation(ivec2 dimensions)
        : DataRepresentation(), dimensions_(dimensions)
    {

    }

    ImageRepresentation::~ImageRepresentation() {}

    void ImageRepresentation::resize(ivec2 dimensions){
        dimensions_ = dimensions;
    }    


} // namespace
