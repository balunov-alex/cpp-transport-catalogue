#include "svg.h"

namespace svg {

using namespace std::literals;

void ColorPrinter::operator()(std::monostate) const {
    out << "none"sv;
}
    
void ColorPrinter::operator()(std::string color) const {
    out << color;
}
    
void ColorPrinter::operator()(Rgb color) const {
    out << "rgb("sv << static_cast<int>(color.red) << ',' 
                    << static_cast<int>(color.green) << ',' 
                    << static_cast<int>(color.blue) << ')';
}
    
void ColorPrinter::operator()(Rgba color) const {
    out << "rgba("sv << static_cast<int>(color.red) << ',' 
                     << static_cast<int>(color.green) << ',' 
                     << static_cast<int>(color.blue) << ',' 
                     << color.opacity << ')';
}    

std::ostream& operator<<(std::ostream& output, Color color) {
    std::visit(ColorPrinter{output}, color);
    return output;
}
    
bool svg::Point::operator==(const Point& other) const {
    return x == other.x && y == other.y;
}
    
bool svg::Point::operator!=(const Point& other) const {
    return !(*this == other);
}      
    
std::ostream& operator<<(std::ostream& output, StrokeLineCap line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT: output << "butt"sv; break;
        case StrokeLineCap::ROUND: output << "round"sv; break;
        case StrokeLineCap::SQUARE: output << "square"sv; break;    
    }
    return output;
}
    
std::ostream& operator<<(std::ostream& output, StrokeLineJoin line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS: output << "arcs"sv; break;
        case StrokeLineJoin::BEVEL: output << "bevel"sv; break;
        case StrokeLineJoin::MITER: output << "miter"sv; break;
        case StrokeLineJoin::MITER_CLIP: output << "miter-clip"sv; break; 
        case StrokeLineJoin::ROUND: output << "round"sv; break; 
    }
    return output;
}    
    
// ---------- Object ------------------    
    
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------    
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    size_t iteration = 1;
    for (auto point : points_) {
        out << point.x << ',' << point.y;
        if (iteration < points_.size()) {
            out << ' ';    
        }
        ++iteration;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}
    
// ---------- Text ------------------     
    
Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}
    
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}
    
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}
    
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}
    
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}
    
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void PreprocessingText(std:: string_view text, std::ostream& output) {
    for (char c : text) {
        switch (c) {
            case '"':
                output << "&quot;"sv; break;
            case '<':
                output << "&lt;"sv; break;
            case '>':
                output << "&gt;"sv; break;
            case '&':
                output << "&amp;"sv; break;
            case '\'':
                output << "&apos;"sv; break;
            default:
                output << c;
        }
    }    
}    
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);        
    out << " x=\""sv << position_.x << "\""sv
        << " y=\""sv << position_.y << "\""sv
        << " dx=\""sv << offset_.x << "\""sv
        << " dy=\""sv << offset_.y << "\""sv
        << " font-size=\""sv << font_size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;        
    }
    out << ">"sv;
    PreprocessingText(data_, out);
    out << "</text>"sv;
}
    
// ---------- Document ------------------
    
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}    
    
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    for (const auto& obj : objects_) {
        obj->Render(RenderContext{out, 2, 2});
    }
    out << "</svg>"sv;
}    

}  // namespace svg
