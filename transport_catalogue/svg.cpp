#include "svg.h"

namespace svg {

    using namespace std::literals;

    Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

    Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op) : red(r), green(g), blue(b), opacity(op) {}

    void ColorPrinter::operator()(std::monostate) const {
        out << "none"s;
    }

    void ColorPrinter::operator()(const std::string& color) const {
        out << color;
    }

    void ColorPrinter::operator()(Rgb color) const {
        out << "rgb("s << static_cast<unsigned>(color.red) << ',' << static_cast<unsigned>(color.green) << ',' << static_cast<unsigned>(color.blue) << ')';
    }

    void ColorPrinter::operator()(Rgba color) const {
        out << "rgba("s << static_cast<unsigned>(color.red) << ',' << static_cast<unsigned>(color.green) << ',' << static_cast<unsigned>(color.blue) << ',' << color.opacity << ')';
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap value) {
        switch (value) {
            case StrokeLineCap::BUTT:
                out << "butt";
                break;
            case StrokeLineCap::ROUND:
                out << "round";
                break;
            case StrokeLineCap::SQUARE:
                out << "square";
                break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin value) {
        switch (value) {
            case StrokeLineJoin::ARCS:
                out << "arcs";
                break;
            case StrokeLineJoin::BEVEL:
                out << "bevel";
                break;
            case StrokeLineJoin::MITER:
                out << "miter";
                break;
            case StrokeLineJoin::MITER_CLIP:
                out << "miter-clip";
                break;
            case StrokeLineJoin::ROUND:
                out << "round";
                break;
        }
        return out;
    }

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
        RenderAttrs(context.out);
        out << "/>"sv;
    }

// ---------- PolyLine ------------------

    Polyline &Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            out << points_[i].x << ',' << points_[i].y;
            if (i != points_.size() - 1) {
                out << " "sv;
            }
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

// ---------- Text ------------------

    Text &Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ =  offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data) {
        data_ = move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\"" << position_.y << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" font-size=\"" << size_ << "\"";
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        RenderAttrs(context.out);
        out << '>';
        for (const auto& symbol : data_) {
            if (symbol == '"') {
                out << "&quot;";
            } else if (symbol == '\'') {
                out << "&apos;";
            } else if (symbol == '<') {
                out << "&lt;";
            } else if (symbol == '>') {
                out << "&gt;";
            } else if (symbol == '&') {
                out << "&amp;";
            } else {
                out << symbol;
            }
        }
        out << "</text>"sv;
    }

// ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object> &&obj) {
        objects_.push_back(move(obj));
    }

    void Document::Render(std::ostream &out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace svg