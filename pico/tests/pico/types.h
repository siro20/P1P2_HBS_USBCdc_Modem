typedef long absolute_time_t;

absolute_time_t make_timeout_time_us(long timeout);
bool time_reached(absolute_time_t time);
