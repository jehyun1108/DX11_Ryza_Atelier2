#pragma once

// 1. TTF 파일읽어서
// 2. stb_truetype 으로 Font 초기화하고
// 3. 원하는 픽셀 높이로 스케일 계산
// 4. stb_rect_pack + stbtt_PackFontRange 로 512 x 512 알파 아틀라스에 글리프들 꽉꽉 채워넣고
// 5. 그걸 RGBA8 버퍼로 만들어 둔 상태

// TTF -> 메모리 상의 폰트 텍스처 + 기본 메트릭 까지