#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <SFML/system.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

const unsigned int width = 800;
const unsigned int height = 600;
const float boxSize = 28.0f;
const float boxBoarder = 6.0f;
const float boxOffset = 2.0f * boxBoarder + boxSize;
const int largeDistance = 3000000;
void draw_box(int i, int j, sf::Color color, sf::RenderWindow& window) {
    sf::RectangleShape shape({ boxSize,boxSize });
    shape.setPosition({ i * boxOffset + boxBoarder,j * boxOffset + boxBoarder });
    shape.setFillColor(color);
    window.draw(shape);
}

struct Node
{
    /*
    Node structure contains the following
            -> the shortest path from the starting point to itself
            -> the node previous to it in the said path
            -> its id which in this case is its memory address
    */
    Node* preNodeptr = nullptr;
    int shortestDistance = largeDistance; // -1 means unitialized
    bool visited = false;
    bool destination = false;
    
    bool setShortestDistance(Node* nodeptr, int Distance) {
        if (shortestDistance == -1 || shortestDistance > Distance) {
            shortestDistance = Distance;
            preNodeptr = nodeptr;
            return true;
        }
        return false;
    }



    
};




enum class Boxtype {
    SOLID,
    FREE,
    VISITED,
    START,
    END,
    PATH
};

struct BoxNode : public Node {
    int x_position = 0;
    int y_position = 0;
    std::vector<BoxNode*> neighbours;
    sf::Color col = sf::Color(0, 0, 255);
private:
    Boxtype type = Boxtype::FREE;
public:
    BoxNode() {};
    BoxNode(int x, int y) :
        x_position(x), y_position(y)
    {}
    void settype(Boxtype t) {
        
        
        switch (t) {
        case Boxtype::FREE:
            col = sf::Color(0, 0, 255);
            break;
        case Boxtype::SOLID:
            col = sf::Color(200, 200, 200);
            break;
        case Boxtype::VISITED:
            if (type != Boxtype::START) {
                col = sf::Color(255, 255, 0);
                
            }
            visited = true;
            break;
        case Boxtype::END:
            col = sf::Color(0, 255, 0);
            destination = true;
            break;
        case Boxtype::START:
            col = sf::Color(255, 0, 255);
            break;
        case Boxtype::PATH:
            if(type != Boxtype::END)
                col = sf::Color(255, 0, 0);

        }
        type = t;

    };

    bool isFree() {
        if (type == Boxtype::SOLID)
            return false;
        return true;
    }

    void draw(sf::RenderWindow& window) {
        draw_box(x_position, y_position,col,window);
    }
    void reset() {
        if (type != Boxtype::SOLID) {
            settype(Boxtype::FREE);
        }
        preNodeptr = nullptr;
        shortestDistance = largeDistance;
        visited = false;
        destination = false;
    }
};

struct BoxArraystructure {
    
    BoxNode* boxArray=nullptr;
    BoxNode defaultBox = BoxNode();
    int Xcount;
    int Ycount;
    BoxArraystructure(int width,int height,float boxOffset) {
        Xcount = (int)(width / (boxOffset));
        Ycount = (int)(height / (boxOffset));
        boxArray = new BoxNode[Xcount * Ycount];
        //Populate the array
        for (int i = 0; i < Xcount; i++) {
            for (int j = 0; j < Ycount; j++) {
                boxArray[j * Xcount + i] = BoxNode(i, j);

            }
        }
        // make the edges Solid
        for (int i = 0; i < Xcount; i++) {
            boxArray[i].settype(Boxtype::SOLID);                            //Top Line
            boxArray[(Ycount - 1) * Xcount + i].settype(Boxtype::SOLID);    //Bottom Line

        }
        for (int j = 0; j < Ycount; j++) {
            boxArray[j*Xcount].settype(Boxtype::SOLID);                            //Left Line
            boxArray[j*Xcount+Xcount-1].settype(Boxtype::SOLID);    //Bottom Line

        }
        initialize_neighbours();

    }
    ~BoxArraystructure() {
        std::cout << "Destructor called" << std::endl;
        delete[] boxArray;
    }
    BoxNode& get(int x, int y) {
        if (x < Xcount && x >=0 && y < Ycount && y >=0) {
            return boxArray[y * Xcount + x];
        }
        std::cout << x << y << "Invalid Box" << "Returning default Box";
        return defaultBox;
    }
    void setBoxtype(int x, int y,Boxtype type) {
        if (x > 0 and y > 0 and x < Xcount - 1 and y < Ycount - 1)
            get(x, y).settype(type);

    }
    void draw(sf::RenderWindow& window) {
        for (int i = 0; i < Xcount; i++) {
            for (int j = 0; j < Ycount; j++) {
                boxArray[j * Xcount + i].draw(window);

            }
        }
    }
    void reset() {
        for (int i = 1; i < Xcount - 1; i++) {
            for (int j = 1; j < Ycount - 1; j++) {
                boxArray[j * Xcount + i].reset();
            }
        }
    }
    void initialize_neighbours() {
        for (int i = 1; i < Xcount - 1; i++) {
            for (int j = 1; j < Ycount - 1;  j++) {
                get(i,j).neighbours.clear();
                get(i, j).neighbours.push_back(
                    &get(i-1, j)
                );
                get(i, j).neighbours.push_back(
                    &get(i , j-1)
                );
                get(i, j).neighbours.push_back(
                    &get(i+1, j)
                );
                get(i, j).neighbours.push_back(
                    &get(i, j+1)
                );
                get(i, j).neighbours.push_back(
                    &get(i+1, j + 1)
                );
                get(i, j).neighbours.push_back(
                    &get(i-1, j + 1)
                );
                get(i, j).neighbours.push_back(
                    &get(i+1, j - 1)
                );
                get(i, j).neighbours.push_back(
                    &get(i-1, j - 1)
                );
                
            }
        }
    }

};


class Djikstart{
private:
    BoxNode* start = nullptr;
    std::list<BoxNode*> priorityque;
    BoxNode* end = nullptr;
    BoxArraystructure* Boxes;
    sf::RenderWindow* win;
    sf::VertexArray path = sf::VertexArray(sf::LineStrip);
public:
    Djikstart(BoxArraystructure* boxes, sf::RenderWindow* window) {
        for (int i = 1; i < boxes->Xcount-1; i++) {
            for (int j = 1; j < boxes->Ycount-1; j++) {
                priorityque.push_back(&(boxes->get(i, j)));
                
            }
        }
        Boxes = boxes;
        win = window;
    }
    void Set(BoxNode* startpoint,BoxNode* endpoint) {
        start = startpoint;
        start->settype(Boxtype::START);
        start->setShortestDistance(nullptr,0);
        end = endpoint;
        end->settype(Boxtype::END);
        path.clear();
        priorityque.clear();
        for (int i = 1; i < Boxes->Xcount - 1; i++) {
            for (int j = 1; j < Boxes->Ycount - 1; j++) {
                priorityque.push_back(&(Boxes->get(i, j)));

            }
        }
        
        
        
       
    }
    void Run() {
        if (start == nullptr) {
            std::cout << "set not called" << std::endl;
            return;
        }
        bool Done = false;
        while (!Done) {
            if (priorityque.empty()) {
                break;
            }
            priorityque.sort(
                [](BoxNode* a, BoxNode* b) {
                   
                    
                    return a->shortestDistance < b->shortestDistance;

                }

            );//least element must come first
            
            auto first = priorityque.front();
            if (first->destination) {
                break;
            }
            if (first->shortestDistance == largeDistance) {
                break;
            }
            for (auto neighbour : first->neighbours) {

                if (!(neighbour->visited) && neighbour->isFree() ){
                    neighbour->setShortestDistance(first, first->shortestDistance + 1);
                    if (neighbour->destination) {
                        //Done = true;
                        break;
                    }
                }
            }
            first->settype(Boxtype::VISITED);
            priorityque.pop_front();

            
        }

        BoxNode* current = end;
        while (true) {
            path.append(box_center(current->x_position, current->y_position));
            if (current->preNodeptr == nullptr) {
                break;
            }
            current->settype(Boxtype::PATH);
            current = (BoxNode*)(current->preNodeptr);
            
        }
       
    }
    void draw_path() {
        win->draw(path);
    }
    void reset() {
        start = nullptr;
        end = nullptr;
        path.clear();
    }
    sf::Vector2f box_center(int x, int y) {
        float xpos = x * boxOffset + boxOffset / 2;
        float ypos = y * boxOffset + boxOffset / 2;
        return sf::Vector2f(xpos, ypos);
    }
};




int main()
{
    //opening a window
    sf::RenderWindow window(sf::VideoMode(width, height), "PathFinding",4);

    const int boxNoX = (int)(width / (boxOffset));
    const int boxNoY = (int)(height / (boxOffset));
    

    auto boxes = BoxArraystructure(width, height, boxOffset);

    auto path_finder = Djikstart(&boxes,&window);
    BoxNode* START = &boxes.get(1, 1);
    BoxNode* END = &boxes.get(boxes.Xcount - 2, boxes.Ycount - 2);

    std::list<int> l = { 1,4,3,9,5 };
    int to_be_inserted = 6;
    for (auto val : l) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    for (auto it = l.begin(); it != l.end(); it++) {
        if (*it > to_be_inserted) {
            l.insert(it,to_be_inserted);
            break;
        }
    }
    for (auto val : l) {
        std::cout << val << std::endl;
    }



    //handling user events and drawing
    while (window.isOpen()) {

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                int mouse_X = event.mouseButton.x;
                int mouse_Y = event.mouseButton.y;
                int i = (int)(mouse_X / boxOffset);
                int j = (int)(mouse_Y / boxOffset);
                if (event.mouseButton.button == sf::Mouse::Button::Left)
                    boxes.setBoxtype(i,j,Boxtype::SOLID);
                else if (event.mouseButton.button == sf::Mouse::Button::Right)
                    boxes.setBoxtype(i,j,Boxtype::FREE);
                

            }
            if (event.type == sf::Event::MouseButtonReleased) {
                boxes.reset();
                path_finder.reset();
                //path_finder.Set(&boxes.get(1, 1),&boxes.get(boxes.Xcount - 2, boxes.Ycount - 2));
                path_finder.Set(START, END);
                path_finder.Run();
            }
            if (event.type == sf::Event::KeyPressed) {
                int sx = START->x_position;
                int sy = START->y_position;
                int ex = END->x_position;
                int ey = END->y_position;
                switch (event.key.code) {
                    case sf::Keyboard::Key::W:
                        sy -= 1;
                        break;
                    case sf::Keyboard::Key::S:
                        sy += 1;
                        break;
                    case sf::Keyboard::Key::A:
                        sx -= 1;
                        break;
                    case sf::Keyboard::Key::D:
                        sx += 1;
                        break;
                    case sf::Keyboard::Key::Up:
                        ey -= 1;
                        break;
                    case sf::Keyboard::Key::Down:
                        ey += 1;
                        break;
                    case sf::Keyboard::Key::Left:
                        ex -= 1;
                        break;
                    case sf::Keyboard::Key::Right:
                        ex += 1;
                        break;
                        
                }
                if (sx > 0 && sx < boxes.Xcount - 1 && sy >0 and sy < boxes.Ycount - 1) {
                    START = &boxes.get(sx, sy);
                }
                if (ex > 0 && ex < boxes.Xcount - 1 && ey >0 and ey < boxes.Ycount - 1) {
                    END = &boxes.get(ex, ey);
                }
                boxes.reset();
                path_finder.reset();
                path_finder.Set(START, END);
                path_finder.Run();
                
                   
            }
            


        }

        //clearing and drawing
        window.clear(sf::Color::Black);
  
        boxes.draw(window);
        path_finder.draw_path();
        window.display();
        
    }


    return 0;
}