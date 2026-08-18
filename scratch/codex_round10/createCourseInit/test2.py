from pathlib import Path
import sys
sys.path.insert(0, 'scratch/codex_round10')
import harness
base=Path('scratch/codex_round10/createCourseInit/assembled.cpp').read_text();start=base.index('    // Written as three nested');end=base.index('    daPyDemoMng_c::mspInstance->init();',start)
repl='''    if (action == 0) goto normal_course;
    if (action == 1) goto normal_course;
    if (action == 0x17) goto normal_course;
    makeCourseInList(daPyDemoMng_c::mspInstance);
    pos = getPlayerSetPos(stageField_0x120e(stage), stageField_0x1211(stage));
    u8 flag;
    if (pos.x <= dGameCom::getDispCenterX()) flag = 0; else flag = 1;
    for (int i = 0; i < 4; i++) create(i, &pos, action, flag);
    return;

normal_course:
''';p=Path('scratch/codex_round10/createCourseInit/goto.cpp');p.write_text(base[:start]+repl+base[end:],encoding='ascii');o=p.with_suffix('.o');t=p.with_suffix('.txt');ok,log=harness.compile_draft(str(p),str(o));print(ok,log);harness.disasm(str(o),str(t));print(harness.diff_fn('wip/player_manager/target_text.txt',str(t),'createCourseInit__9daPyMng_cFv')[1])
