#ifndef APP_H
#define APP_H

class App {
public:
  App();
  App(App &&) = default;
  App(const App &) = default;
  App &operator=(App &&) = default;
  App &operator=(const App &) = default;
  ~App();

private:
  
};

#endif // !APP_H

