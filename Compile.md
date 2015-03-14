# Biên dịch chương trình #
## Thư viện yêu cầu ##
Yêu cầu phải có thư viện phát triển libnotify
Trong ubuntu kiểm tra bằng câu lệnh
**$pkg-config --libs --cflags libnotify**
Nếu chưa có
Cài đặt bằng lệnh
$sudo apt-get install libnotify-bin libnotify-dev

## Lệnh compile ##
Sau khi chuyển vào thư mục source code.
Có thể compile bằng 1 trong 2 lệnh sau
> + Để debug các xử lý qua terminal
    * $gcc -g -o NotifyWord NotifyWord.c `pkg-config --cflags --libs libnotify` -D _DEBUG\_MACRO_

> + Không sử dụng debug xử lý nữa
    * gcc -g -o NotifyWord NotifyWord.c `pkg-config --cflags --libs libnotify`