
#include <iostream>
#include <tinyxml2.h>
#include <sstream>
#include "svg_to_png.hpp"
#include "elements.hpp"

using namespace tinyxml2;

namespace svg {
    // Color parsing
    const std::map<std::string, color> COLORS = {
            {"black",  {0,   0,   0}},
            {"white",  {255, 255, 255}},
            {"red",    {255, 0,   0}},
            {"green",  {0,   255, 0}},
            {"blue",   {0,   0,   255}},
            {"yellow", {255, 255, 0}}
    };

    color parse_color(const std::string &str) {
        color c;
        char first_ch = str.at(0);
        if (first_ch == '#') {
            int v;
            std::stringstream ss(str.substr(1));
            ss >> std::hex >> v;
            c.red = (v >> 16);
            c.green = (v >> 8) & 0xFF;
            c.blue = v & 0xFF;
        } else {
            c = COLORS.at(str);
        }
        return c;
    }

    // Transformation parsing

    void parse_transform(shape *s, XMLElement *elem) {
        const char* p_t_attr = elem->Attribute("transform");
        if (p_t_attr == NULL)
            return; // Not defined
        point origin{0,0};
        const char* p_t_o_attr = elem->Attribute("transform-origin");
        if (p_t_o_attr != NULL) {
            std::string str = p_t_o_attr;
            std::stringstream ss(str);
            ss >> origin.x >> origin.y;
        }
        std::string attr(p_t_attr);
        for (char &c: attr) {
            if (c == '(' || c == ')' || c == ',') {
                c = ' ';
            }
        }
        std::stringstream ss(attr);
        std::string type;
        ss >> type;
        if (type == "translate") {
            int x, y;
            ss >> x >> y;
            s->translate({x, y});
        } else if (type == "scale") {
            int v;
            ss >> v;
            s->scale(origin, v);
        } else if (type == "rotate") {
            int v;
            ss >> v;
            s->rotate(origin, v);
        } else {
            std::cout << "Unrecognised transform type: " << type << std::endl;
        }
    }

    // Point list parsing
    void
    parse_points(const std::string &s, std::vector<point> &points) {
        std::stringstream ss(s);
        std::string val;
        while (std::getline(ss, val, ' ')) {
            val.at(val.find(',')) = ' ';
            std::stringstream ss2(val);
            int x, y;
            ss2 >> x >> y;
            points.push_back({x, y});
        }
    }

    // Shape parsing
    ellipse *parse_ellipse(XMLElement *elem) {
        int cx = elem->IntAttribute("cx");
        int cy = elem->IntAttribute("cy");
        int rx = elem->IntAttribute("rx");
        int ry = elem->IntAttribute("ry");
        color fill = parse_color(elem->Attribute("fill"));
        return new ellipse(fill, {cx, cy}, {rx, ry});
    }
    // TODO other parsing functions for elements
    circle *parse_circle (XMLElement *elem){
        int cx=elem->IntAttribute("cx");
        int cy = elem->IntAttribute("cy");
        int r = elem->IntAttribute("r");
        color fill = parse_color(elem->Attribute("fill"));
        return new circle(fill, {cx, cy}, {r, r});
    }
    polygon *parse_polygon(XMLElement *elem){
        std::vector<point> points;
        parse_points(elem->Attribute("points"),  points);
        color fill= parse_color(elem->Attribute("fill"));
        return new polygon(fill,points);
    }
    rect *parse_rect(XMLElement *elem){
        int x=elem->IntAttribute("x");
        int y=elem->IntAttribute("y");
        int width=elem->IntAttribute("width");
        int height=elem->IntAttribute("height");
        color fill= parse_color(elem->Attribute("fill"));
        std::vector<point> points;
       points.push_back({x,y});
        points.push_back({x+width-1,y});
        points.push_back({x+width-1,y+height-1});
        points.push_back({x,y+height-1});


        return new rect(fill,points);
    }


    polyline *parse_polyline(XMLElement *elem){
        std::vector<point> points;
        parse_points(elem->Attribute("points"),  points);
        color stroke= parse_color(elem->Attribute("stroke"));
        return new polyline(stroke,points);
    }


    line *parse_line(XMLElement *elem){
        int x1=elem->IntAttribute("x1");
        int y1=elem->IntAttribute("y1");
        int x2=elem->IntAttribute("x2");
        int y2=elem->IntAttribute("y2");
        color stroke= parse_color(elem->Attribute("stroke"));
        std::vector<point> points;
        points.push_back({x1,y1});
        points.push_back({x2,y2});
        return new line(stroke,points);
    }

    group *parse_group(XMLElement *elem, std::vector<shape *> &shapes);
    // Loop for parsing shapes
    void parse_shapes(XMLElement *elem, std::vector<shape *> &shapes) {
        int cont=0;
        if(elem->FirstChildElement()==NULL){
            std::cout<<" First Child ELEM É NULL "<<std::endl;
        }
        if(elem->NextSiblingElement()==NULL){
            std::cout<<"Next ELEM É NULL "<<std::endl;
        }
        for (auto child_elem = elem->FirstChildElement();
             child_elem != NULL;
             child_elem = child_elem->NextSiblingElement()) {
            std::string type(child_elem->Name());
            shape *s;
            // TODO complete
            if (type == "ellipse") {
                s = parse_ellipse(child_elem);
                std::cout<<" elipse "<<std::endl;

            }
            else if(type=="circle"){
                s = parse_circle(child_elem);
                std::cout<<"circle "<<std::endl;

            }
            else if(type=="polygon"){
                s = parse_polygon(child_elem);
                std::cout<<"polygon :"<<std::endl;
            }
            else if(type=="rect"){
                s= parse_rect(child_elem);
                std::cout<<" rect "<<std::endl;
            }
            else if(type=="polyline"){
                s= parse_polyline(child_elem);
                std::cout<<" polyline "<<std::endl;
            }
            else if(type=="line"){
                s=parse_line(child_elem);
                std::cout<<"line "<<std::endl;

            }
            else if(type=="g"){
             //   std::cout<<cont<<std::endl;
                std::cout<<"g"<<std::endl;
                s= parse_group(child_elem,shapes);
                std::cout<<"/g"<<std::endl;
               // cont++;
                //std::cout<<cont<<std::endl;

            }
            else {
                std::cout << "Unrecognized shape type: " << type << std::endl;
                continue;
            }
           // std::cout<<"Antes Transform\n";
            parse_transform(s, child_elem);
          //  std::cout<<"Depois Transform\n";
            shapes.push_back(s);
         //   std::cout<<"Depois Push_back\n";
        }
    }
    group *parse_group(XMLElement *elem, std::vector<shape *> &shapes) {
        if(elem->FirstChildElement()== NULL){
            std::cout<<"NULL\n";
            if(elem->NextSiblingElement()->FirstChildElement()==NULL){
                std::cout<<"NULL 2 \n";
                parse_shapes(elem,shapes);
            }
            else{
                std::cout<<"NULL 3 \n";
                parse_shapes(elem->NextSiblingElement(),shapes);
            }
        }
        else{
            std::cout<<"ELSE1\n";
     
      parse_shapes(elem,shapes);
        }
        return new group({235, 215, 155}, shapes);
    }

    // Main conversion function.
    // TODO adapt if necessary
    void svg_to_png(const std::string &svg_file, const std::string &png_file) {
        XMLDocument doc;
        XMLError r = doc.LoadFile(svg_file.c_str());
        if (r != XML_SUCCESS) {
            return;
        }
        XMLElement *elem = doc.RootElement();
        std::vector<shape *> shapes;
        parse_shapes(elem, shapes);
        int width = elem->IntAttribute("width");
        int height = elem->IntAttribute("height");
        png_image img(width, height);
        for (auto s: shapes) {
            s->draw(img);
        }
        img.save(png_file);
        for (auto s: shapes) {
            delete s;
        }
    }

}
