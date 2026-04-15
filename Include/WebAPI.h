#ifndef WEBAPI_H
#define WEBAPI_H

class WebAPI {
public:
  WebAPI();
  WebAPI(WebAPI &&) = default;
  WebAPI(const WebAPI &) = default;
  WebAPI &operator=(WebAPI &&) = default;
  WebAPI &operator=(const WebAPI &) = default;
  ~WebAPI();

private:
  
};

#endif // !WEBAPI_H
