# Migano


![migan_bw_line](https://user-images.githubusercontent.com/26345945/172053377-39f2c346-14c0-4ff4-9e16-e84352e673b8.jpg)

이것은 제 미간이 아니라 피아노입니다.

트위치 스트리머 [김나성](https://twitch.tv/naseongkim)의 음성 소스를 샘플로 사용하는 가상 악기 플러그인입니다.

프로그램 자체는 간단한 샘플러 플러그인이고, 사운드뱅크를 프로세싱하여 불러와서 사용할 수 있습니다.



#### 설명 (일반)

- 오른쪽 Releases 탭에서 프로그램을 다운로드할 수 있습니다.

- Migano.exe를 실행하면 악기를 연주할 수 있습니다.

  - 재생할 preset을 자동으로 해당 exe 파일이 있는 경로에서 찾습니다.

    MiganoSoundBank 폴더가 있어야 하고, 그 안에 있는 모든 msb 파일을 로드합니다.

    쉽게 말해서, 그냥 안 건드리면 됩니다.

  - 컴퓨터 키보드 및 Midi 장치로 재생할 수 있습니다.

  - 기본 옥타브는 C5입니다. 원래는 C4여야 하는데, 제가 쓰는게 FL Studio라서 C5입니다.

    실제 재생할 수 있는 건반은 C0 ~ G10까지 128개입니다. 무슨 말인지 모르겠다면, 몰라도 됩니다.

  - 사용하는 키보드 버튼은 다음과 같습니다.

    ![migano_keyboard](https://user-images.githubusercontent.com/26345945/172053403-73893f2e-8e4f-4d80-a46a-82a6838494ed.jpg)

    Q ~ ]키와 1~=키 : 대응하는 건반

    z키 : 옥타브 1칸 내림

    x키 : 옥타브 1칸 올림

  

  -  ![migano](https://user-images.githubusercontent.com/26345945/172053392-a9b99d79-cd06-4555-be47-14067286e843.png)

    Attack : 키를 눌렀을 때, 볼륨이 0에서 1까지 올라가는 데 걸리는 시간입니다.

    Decay : attack 단계에서 볼륨이 1까지 올라간 뒤에 sustain까지 내려가는 데 걸리는 시간입니다.

    Sustain : 키를 누르는 중에 유지할 볼륨의 수치입니다.

    Release : 키를 뗀 후에, 볼륨이 sustain에서 0까지 내려가는 데 걸리는 시간입니다.

     

    Start : 샘플 재생을 시작할 지점입니다.

    End : 샘플 재생을 끝낼 지점입니다.

    

    Use Loop : 루프를 사용할 지 여부입니다. 사용하면 누르는 도중에 루프 구간을 반복 재생합니다.

    Loop Start : 루프 구간의 시작 지점입니다.

    Loop End : 루프 구간의 끝 지점입니다.

    Resamp Loop : 루프 구간을 Resampling하여 재생할 지 여부입니다.

    별 건 아니고, 루프 구간이 너무 짧으면 음이 안 올라가서 땜빵하려고 만든 옵션입니다.

    

    Preset : 재생할 샘플을 선택할 수 있습니다.

    

- tool 폴더에 있는 Preprocessing Tool을 이용해서 msb 파일을 만들거나 수정할 수 있습니다.

  - wav / mp3 / ogg 파일 등을 불러와서도 만들 수 있고, 불러올 수 있는 소스의 길이는 5초로 제한했습니다.

    재생은 C5 ~ G6 까지만 지원합니다. 128개 다 하면 너무 렉 걸림

    msb 파일로 저장할 때는 각 건반에 대한 샘플을 128개 모두 생성해서 저장합니다.

    기존 소스가 마음에 안들거나 내가 새로 소스를 만들겠다 그런 게 아니면, 어지간하면 얘는 건드릴 일 없을 겁니다.

  

  - 대부분 실제 악기랑 똑같고, Semitone을 추가했습니다.

    Semitone은 드래그가 끝나고 마우스를 뗐을 때 적용합니다. 실시간은 렉 엄청 걸림

    Root Note는 기본음의 위치입니다. 건반을 우클릭하여 지정할 수 있습니다.

  

- 버그 제보는 Issue에 해주시면 해결될 수도, 안 될 수도 있습니다. 양해 바랍니다.



#### 설명 (DAW / MIDI)

DAW (작곡 프로그램)에 사용할 분들을 위한 추가 설명입니다.

- Migano.dll (64비트) 혹은 Migano_x86.dll (32비트)를 DAW 프로그램에서 사용하는 VST2 폴더에 붙여넣으시고 로드하시면 됩니다.

  Migano.vst3를 VST3 폴더에 붙여넣으셔도 됩니다.

  두 경우 모두 사용할 MiganoSoundBank 폴더를 같은 위치에 넣어주세요.

- vst에는 옥타브 올림/내림 키가 없습니다.

- Pitch Wheel을 만들다 말았습니다. Use Loop, Resample Loop가 모두 켜져 있을 때만 작동합니다.

  다른 경우를 모두 되게 만드려니까 실시간으로 Pitch Shift를 적용해야 되는데, 너무 렉이 심해요.

  그래서 Pitch Bend Level을 구현 안 했습니다. 이거 할 때쯤 현타왔음;



- Test Environment : FL Studio 20.9.2, VST2 64bit, Windows 10



#### Libraries

- [JUCE](https://juce.com/) Framework
- [SoundTouch](https://www.surina.net/soundtouch/) Library (for Pitch Shift)

