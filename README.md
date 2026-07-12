# Computer Network Socket Programming

C 언어와 POSIX Socket API를 이용한 TCP/UDP 네트워크 프로그래밍 실습 및 과제 모음입니다. 기본적인 소켓 연결부터 파일 전송, I/O 멀티플렉싱, 멀티스레드 서버, HTTP 통신까지 단계적으로 다룹니다.

## 학습 내용

- TCP와 UDP 소켓 통신
- 클라이언트/서버 구조
- 파일 업로드 및 다운로드
- 소켓 옵션과 주소 재사용
- `select()` 기반 I/O 멀티플렉싱
- POSIX Thread 기반 동시성 처리
- HTTP 요청 및 응답 처리
- 연결 중단 후 파일 전송 재개

## 프로젝트 구성

| 폴더 | 주제 | 설명 |
| --- | --- | --- |
| [`practice1`](practice1/README.md) | TCP Echo | 클라이언트의 메시지를 그대로 반환하는 기본 TCP 서버/클라이언트 |
| [`practice2`](practice2/README.md) | HTTP Client | 도메인 주소 변환과 HTTP/1.0 GET 요청 처리 |
| [`practice3`](practice3/README.md) | TCP File Upload | TCP를 이용한 파일 이름 및 파일 데이터 전송 |
| [`practice4`](practice4/README.md) | UDP File Transfer | UDP를 이용한 이미지 파일 전송 |
| [`practice5`](practice5/README.md) | `SO_REUSEADDR` | 서버 주소와 포트를 빠르게 재사용하는 소켓 옵션 |
| [`practice6`](practice6/README.md) | `select()` Server | I/O 멀티플렉싱을 이용한 다중 클라이언트 파일 서버 |
| [`practice7`](practice7/README.md) | Thread Server | 클라이언트별 스레드를 사용하는 동시 파일 서버 |
| [`practice8`](practice8/README.md) | HTTP Server | 정적 파일을 제공하는 멀티스레드 HTTP/1.0 서버 |
| [`hw3`](hw3/README.md) | Resumable Upload | 연결이 끊겨도 이어서 전송할 수 있는 TCP 파일 업로더 |

## 학습 흐름

실습은 기본 TCP 통신에서 시작해 파일 전송과 UDP 통신으로 확장됩니다. 이후 소켓 옵션, `select()`, POSIX Thread를 통해 서버의 안정성과 동시 처리 방식을 학습하고, 마지막으로 HTTP 서버와 중단 후 재개 가능한 파일 업로드를 구현합니다.

```text
TCP 기초 -> HTTP 클라이언트 -> 파일 전송 -> UDP
         -> 소켓 옵션 -> select() -> pthread -> HTTP 서버
         -> 중단 후 재개 가능한 파일 업로드
```

## 요구 환경

- Linux 또는 macOS 등 POSIX 호환 운영체제
- GCC 또는 Clang
- POSIX Socket API
- POSIX Threads (`practice7`, `practice8`, `hw3`)
- `make` (`hw3`)

## 컴파일

각 실습 폴더로 이동한 뒤 필요한 소스 파일을 직접 컴파일할 수 있습니다.

```bash
gcc -Wall -Wextra -o server server_source.c
gcc -Wall -Wextra -o client client_source.c
```

스레드를 사용하는 프로그램에는 `-pthread` 옵션을 추가합니다.

```bash
gcc -Wall -Wextra -pthread -o server server_source.c
```

`hw3`는 제공된 Makefile로 서버와 클라이언트를 함께 빌드할 수 있습니다.

```bash
cd hw3
make
```

정확한 소스 파일 이름과 프로그램 동작은 각 폴더의 README를 참고하세요.

## 실행 방식

대부분의 서버는 포트 번호를, 클라이언트는 서버 IP와 포트 번호를 명령행 인자로 받습니다.

```bash
./server <port>
./client <server_ip> <port>
```

파일 이름처럼 추가 인자가 필요한 프로그램도 있으므로 실행 전 소스의 `Usage` 메시지나 해당 폴더의 README를 확인해야 합니다. 서버를 먼저 실행한 다음 별도의 터미널에서 클라이언트를 실행합니다.

## 참고 사항

이 저장소는 네트워크 프로그래밍 학습을 위한 예제 코드입니다. 일부 프로그램은 오류 처리, 입력 검증, 패킷 손실 복구, 보안 기능이 제한적이므로 실제 서비스 환경보다는 로컬 실습 환경에서 사용하는 것을 권장합니다.
