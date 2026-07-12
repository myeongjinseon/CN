# Practice 4 - UDP File Transfer

UDP 데이터그램을 이용해 서버의 `hgu.jpg` 파일을 클라이언트로 전송하는 예제입니다.
서버가 파일 크기와 데이터를 보내면 클라이언트는 이를 `recv.jpg`로 저장하고 수신 완료 메시지를 보냅니다.
