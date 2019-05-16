#include <steem/plugins/sig_by_key_api/sig_by_key_api.hpp>
#include <steem/plugins/sig_by_key_api/sig_by_key_api_plugin.hpp>
#include <steem/plugins/sig_by_key_api/HibeGS.hpp>
#include <iostream>
using namespace relicxx;
using namespace forwardsec;
namespace steem
{
namespace plugins
{
namespace sig_by_key
{

namespace detail
{

class sig_by_key_api_impl
{
public:
  relicResourceHandle relic;
  PairingGroup group;
  MasterPublicKey mpk;
  relicxx::G2 msk;
  relicxx::G2 t11;
  relicxx::G1 t21;
  relicxx::G2 t31;
  relicxx::GT t41;
  //如果需要新建多个群，则需要修改代码，改为获取对应群组的私钥
  //目前的需求只存在一个群组，所以无需修改
  //同一个群，提交论文和审稿对应的群公私钥是同一对
  GroupSecretKey gsk;
  UserSecretKey usk;

  sig_by_key_api_impl() {}
  ~sig_by_key_api_impl() {}

  set_group_return set_group(const set_group_args &args)
  {
    set_up();
    groupSetup(args.groupID, msk, gsk, mpk);
    //返回gsk给group manager,先假设只返回给一个人
    set_group_return final;
    //此处需保存每个group的私钥
    final.a0 = g2ToBin(gsk.a0);
    final.a2 = g2ToBin(gsk.a2);
    final.a3 = g2ToBin(gsk.a3);
    final.a4 = g2ToBin(gsk.a4);
    final.a5 = g1ToBin(gsk.a5);
    return final;
  }
  join_group_return join_group(const join_group_args &args)
  {
    set_up();
    string groupID = args.groupID;
    string userID = args.userID;

    join(groupID, userID, gsk, usk, mpk);
    join_group_return final;

    final.b0 = g2ToBin(usk.b0);
    final.b3 = g2ToBin(usk.b3);
    final.b4 = g2ToBin(usk.b4);
    final.b5 = g1ToBin(usk.b5);
    return final;
  }
  // 返回用户签名
  get_sig_return get_sig(const get_sig_args &args)
  {
    set_up();
    get_sig_return final;
    UserSecretKey usk;
    usk.b0 = binToG2(args.b0);
    usk.b3 = binToG2(args.b3);
    usk.b4 = binToG2(args.b4);
    usk.b5 = binToG1(args.b5);
    Signature sig;
    relicxx::ZR m = group.hashListToZR(args.m);
    sign(args.groupID, args.userID, m, usk, sig, mpk);

    final.c0 = g2ToBin(sig.c0);
    final.c5 = g1ToBin(sig.c5);
    final.c6 = g2ToBin(sig.c6);
    final.e1 = g1ToBin(sig.e1);
    final.e2 = g2ToBin(sig.e2);
    final.e3 = gtToBin(sig.e3);
    final.c = zrToStr(sig.c);
    final.s1 = zrToStr(sig.s1);
    final.s2 = zrToStr(sig.s2);
    final.s3 = zrToStr(sig.s3);
    return final;
  }
  open_paper_return open_paper(const open_paper_args args)
  {
    set_up();
    open_paper_return final;
    Signature sig;
    sig.c0 = binToG2(args.c0);
    sig.c5 = binToG1(args.c5);
    sig.c6 = binToG2(args.c6);
    sig.e1 = binToG1(args.e1);
    sig.e2 = binToG2(args.e2);
    sig.e3 = binToGT(args.e3);
    sig.c = strToZR(args.c);
    sig.s1 = strToZR(args.s1);
    sig.s2 = strToZR(args.s2);
    sig.s3 = strToZR(args.s3);
    final.result = open(mpk, gsk, sig, args.userID);
    return final;
  }
  verify_user_return verify_user(const verify_user_args args)
  {
    set_up();
    verify_user_return final;
    Signature sig;
    sig.c0 = binToG2(args.c0);
    sig.c5 = binToG1(args.c5);
    sig.c6 = binToG2(args.c6);
    sig.e1 = binToG1(args.e1);
    sig.e2 = binToG2(args.e2);
    sig.e3 = binToGT(args.e3);
    sig.c = strToZR(args.c);
    sig.s1 = strToZR(args.s1);
    sig.s2 = strToZR(args.s2);
    sig.s3 = strToZR(args.s3);

    final.result = verify(group.hashListToZR(args.m), sig, args.groupID, mpk);
    return final;
  }
  test_return test(const test_args &args)
  {
    test_return final;
    setup(mpk, msk);
    string groupID = "science";
    const set_group_args set_args{.groupID = groupID};
    set_group_return sgr = set_group(set_args);

    GroupSecretKey gsk2;
    gsk2.a0 = binToG2(sgr.a0);
    gsk2.a2 = binToG2(sgr.a2);
    gsk2.a3 = binToG2(sgr.a3);
    gsk2.a4 = binToG2(sgr.a4);
    gsk2.a5 = binToG1(sgr.a5);

    const join_group_args join_args{.groupID = "science", .userID = "www"};
    join_group_return jgr = join_group(join_args);
    UserSecretKey usk2;

    usk2.b0 = binToG2(jgr.b0);
    usk2.b3 = binToG2(jgr.b3);
    usk2.b4 = binToG2(jgr.b4);
    usk2.b5 = binToG1(jgr.b5);

    string str = "123";
    get_sig_args sig_args{.groupID = "science", .userID = "www", .m = str, .b0 = jgr.b0, .b3 = jgr.b3, .b4 = jgr.b4, .b5 = jgr.b5};
    get_sig_return gsr = get_sig(sig_args);
    Signature sig;

    sig.c0 = binToG2(gsr.c0);
    sig.c5 = binToG1(gsr.c5);
    sig.c6 = binToG2(gsr.c6);
    sig.e1 = binToG1(gsr.e1);
    sig.e2 = binToG2(gsr.e2);
    sig.e3 = binToGT(gsr.e3);
    sig.c = strToZR(gsr.c);
    sig.s1 = strToZR(gsr.s1);
    sig.s2 = strToZR(gsr.s2);
    sig.s3 = strToZR(gsr.s3);

    open_paper_args open_args{.userID = "www", .c0 = gsr.c0, .c5 = gsr.c5, .c6 = gsr.c6, .e1 = gsr.e1, .e2 = gsr.e2, .e3 = gsr.e3, .c = gsr.c, .s1 = gsr.s1, .s2 = gsr.s2, .s3 = gsr.s3};
    open_paper_return opr = open_paper(open_args);

    verify_user_args ver_args{.groupID = "science", .m = str, .c0 = gsr.c0, .c5 = gsr.c5, .c6 = gsr.c6, .e1 = gsr.e1, .e2 = gsr.e2, .e3 = gsr.e3, .c = gsr.c, .s1 = gsr.s1, .s2 = gsr.s2, .s3 = gsr.s3};
    if (verify_user(ver_args).result == true && opr.result == true)
      final.result = "true";
    else
      final.result = "false";

    /* GroupSecretKey gsk;
    UserSecretKey usk;
    Signature sig;
    const relicxx::ZR m = group.randomZR();
    groupSetup("science", msk, gsk, mpk);
    join("science", "www", gsk, usk, mpk);
    sign(m, usk, sig, mpk);
    verify(m, sig, "science", mpk);
    if (open(mpk, gsk, sig)  && verify(m, sig, "science", mpk))
      final.result = "true";
    else
    {
      final.result = "false";
    }
  */
    return final;
  }

private:
  void setup(MasterPublicKey &mpk, relicxx::G2 &msk) const
  {
    const unsigned int l = 4;
    ZR alpha = group.randomZR();
    mpk.g = group.randomG1();
    mpk.g2 = group.randomG2();
    mpk.hibeg1 = group.exp(mpk.g, alpha);
    //we setup four level HIBE here,the first level is Group identity,the second level is user identity
    //the third level is the signed message,the last level is a random identity
    mpk.l = 4;
    for (unsigned int i = 0; i <= l; i++)
    {
      ZR h = group.randomZR();
      mpk.hG2.push_back(group.exp(mpk.g2, h));
    }
    mpk.n = group.randomGT();
    msk = group.exp(mpk.g2, alpha);
  }
  void groupSetup(const std::string &groupID, const G2 &msk, GroupSecretKey &gsk, const MasterPublicKey &mpk) const
  {
    const ZR e = group.hashListToZR(groupID);
    const ZR r1 = group.randomZR();
    gsk.a0 = group.exp(group.mul(mpk.hG2.at(0), group.exp(mpk.hG2.at(1), e)), r1);
    gsk.a0 = group.mul(msk, gsk.a0);
    gsk.a2 = group.exp(mpk.hG2.at(2), r1);
    gsk.a3 = group.exp(mpk.hG2.at(3), r1);
    gsk.a4 = group.exp(mpk.hG2.at(4), r1);
    gsk.a5 = group.exp(mpk.g, r1);
  }
  void join(const string &groupID, const string &userID, const GroupSecretKey &gsk, UserSecretKey &usk, const MasterPublicKey &mpk) const
  {

    const ZR gUserID = group.hashListToZR(userID);
    const ZR gGroupID = group.hashListToZR(groupID);
    const ZR r2 = group.randomZR();

    relicxx::G2 res = group.mul(mpk.hG2.at(0), group.exp(mpk.hG2.at(1), gGroupID));
    res = group.exp(group.mul(res, group.exp(mpk.hG2.at(2), gUserID)), r2);
    usk.b0 = group.mul(gsk.a0, group.exp(gsk.a2, gUserID));
    usk.b0 = group.mul(usk.b0, res);
    usk.b3 = group.mul(gsk.a3, group.exp(mpk.hG2.at(3), r2));
    usk.b4 = group.mul(gsk.a4, group.exp(mpk.hG2.at(4), r2));
    usk.b5 = group.mul(gsk.a5, group.exp(mpk.g, r2));
  }
  void sign(string groupID, string userID, const ZR &m, const UserSecretKey &usk, Signature &sig, const MasterPublicKey &mpk)
  {
    const ZR gUserID = group.hashListToZR(userID);
    const ZR gGroupID = group.hashListToZR(groupID);
    //G(UserID),G(r4),k are public
    const ZR r3 = group.randomZR();
    //r4 use to blind identity
    const ZR r4 = group.randomZR();
    //used to encrypt identity to the group manager
    const ZR k = group.randomZR();
    relicxx::G2 res = group.mul(mpk.hG2.at(0), group.exp(mpk.hG2.at(1), gGroupID));
    res = group.mul(res, group.exp(mpk.hG2.at(2), gUserID));
    res = group.mul(res, group.exp(mpk.hG2.at(3), m));
    res = group.exp(group.mul(res, group.exp(mpk.hG2.at(4), r4)), r3);
    sig.c0 = group.mul(usk.b0, group.exp(usk.b3, m));
    sig.c0 = group.mul(group.mul(sig.c0, group.exp(usk.b4, r4)), res);
    sig.c5 = group.mul(usk.b5, group.exp(mpk.g, r3));

    sig.c6 = group.mul(group.exp(mpk.hG2.at(2), gUserID), group.exp(mpk.hG2.at(4), r4));
    sig.e1 = group.exp(mpk.g, k);
    sig.e2 = group.exp(group.mul(mpk.hG2.at(0), group.exp(mpk.hG2.at(1), gGroupID)), k);

    sig.e3 = group.exp(group.pair(mpk.g2, mpk.hibeg1), k);
    sig.e3 = group.mul(sig.e3, group.exp(mpk.n, gUserID));

    ZR k1 = group.randomZR();
    ZR k2 = group.randomZR();
    ZR k3 = group.randomZR();
    relicxx::G2 t1 = group.mul(group.exp(mpk.hG2.at(2), k1), group.exp(mpk.hG2.at(4), k2));
    relicxx::G1 t2 = group.exp(mpk.g, k3);
    relicxx::G2 f = group.mul(mpk.hG2.at(0), group.exp(mpk.hG2.at(1), gGroupID));
    relicxx::GT gt = group.pair(mpk.hibeg1, mpk.g2);
    relicxx::G2 t3 = group.exp(f, k3);
    relicxx::GT t4 = group.mul(group.exp(mpk.n, k1), group.exp(gt, k3));
    cout << t1;
    ZR c = group.hashListToZR(groupID + g2ToStr(mpk.hG2.at(0)) +
                              g2ToStr(mpk.hG2.at(1)) + g2ToStr(mpk.hG2.at(2)) +
                              g2ToStr(mpk.hG2.at(4)) + g1ToStr(mpk.g) + g2ToStr(f) +
                              gtToStr(mpk.n) + gtToStr(gt) + g2ToStr(sig.c6) + g1ToStr(sig.e1) + g2ToStr(sig.e2) +
                              gtToStr(sig.e3) + g2ToStr(t1) + g1ToStr(t2) + g2ToStr(t3) + gtToStr(t4));

    sig.c = c;
    sig.s1 = k1 + group.mul(c, gUserID);
    sig.s2 = k2 + group.mul(c, r4);
    sig.s3 = k3 + group.mul(c, k);
    
    t11 = group.mul(group.mul(group.exp(mpk.hG2.at(2), sig.s1), group.exp(mpk.hG2.at(4), sig.s2)), group.exp(sig.c6, -sig.c));
    t21 = group.mul(group.exp(mpk.g, sig.s3), group.exp(sig.e1, -sig.c));
    t31 = group.mul(group.exp(f, sig.s3), group.exp(sig.e2, -sig.c));
    t41 = group.mul(group.mul(group.exp(mpk.n, sig.s1), group.exp(gt, sig.s3)), group.exp(sig.e3, -sig.c));
    cout << sig.c << sig.s1 << sig.s2 << sig.s3 << endl;
    cout << t11;
    /*relicxx::G1 g = group.randomG1();
    cout << g << endl;
    cout << g.g << endl;

    g1ToStr(g);
    uint8_t bin[len];
    for (int i = 0; i < data.size(); i++)
    {
      cout << (unsigned int)data[i];
      bin[i] = (unsigned int)data[i];
    }
    cout << endl;
    relicxx::G1 g2 = g.setBytes(bin);
    cout << "g2";
    cout << g2 << endl;
    cout << "g2.g";
    cout << g2.g;
    if (g1_cmp(g.g, g2.g) == CMP_EQ)
      cout << "hello";
    else
      cout << "bye"; */
  }
  bool verify(const ZR &m, const Signature &sig, const string &groupID, const MasterPublicKey &mpk)
  {
    const ZR gGroupID = group.hashListToZR(groupID);
    const ZR t = group.randomZR();
    const GT M = group.randomGT();
    relicxx::G1 d1 = group.exp(mpk.g, t);
    relicxx::G2 d2 = group.mul(mpk.hG2.at(0), group.exp(mpk.hG2.at(1), gGroupID));
    d2 = group.exp(group.mul(d2, group.mul(group.exp(mpk.hG2.at(3), m), sig.c6)), t);
    relicxx::GT delta3 = group.mul(M, group.exp(group.pair(mpk.hibeg1, mpk.g2), t));
    relicxx::GT result = group.mul(delta3, group.div(group.pair(sig.c5, d2), group.pair(d1, sig.c0)));
    //pok verify

    relicxx::G2 t1 = group.mul(group.mul(group.exp(mpk.hG2.at(2), sig.s1), group.exp(mpk.hG2.at(4), sig.s2)), group.exp(sig.c6, -sig.c));
    relicxx::G1 t2 = group.mul(group.exp(mpk.g, sig.s3), group.exp(sig.e1, -sig.c));
    relicxx::G2 f = group.mul(mpk.hG2.at(0), group.exp(mpk.hG2.at(1), gGroupID));
    relicxx::GT gt = group.pair(mpk.hibeg1, mpk.g2);
    relicxx::G2 t3 = group.mul(group.exp(f, sig.s3), group.exp(sig.e2, -sig.c));
    relicxx::GT t4 = group.mul(group.mul(group.exp(mpk.n, sig.s1), group.exp(gt, sig.s3)), group.exp(sig.e3, -sig.c));
    //需要分析t1和t11相等却输出不同的原因
    cout << sig.c << sig.s1 << sig.s2 << sig.s3 << endl;
    cout << (t1 == t11) << (t2 == t21) << (t3 == t31) << (t4 == t41) << endl;
    cout << t1;
    cout << t11;
    ZR c = group.hashListToZR(groupID + g2ToStr(mpk.hG2.at(0)) +
                              g2ToStr(mpk.hG2.at(1)) + g2ToStr(mpk.hG2.at(2)) +
                              g2ToStr(mpk.hG2.at(4)) + g1ToStr(mpk.g) + g2ToStr(f) +
                              gtToStr(mpk.n) + gtToStr(gt) + g2ToStr(sig.c6) + g1ToStr(sig.e1) + g2ToStr(sig.e2) +
                              gtToStr(sig.e3) + g2ToStr(t1) + g1ToStr(t2) + g2ToStr(t3) + gtToStr(t4));

    //return M == result && sig.c == c;
    return M == result;
  }

  bool open(const MasterPublicKey &mpk, const GroupSecretKey &gsk, const Signature &sig, string userID)
  {
    const ZR gUserID = group.hashListToZR(userID);
    relicxx::GT t = group.div(group.pair(sig.e1, gsk.a0), group.pair(gsk.a5, sig.e2));
    //goes through all user identifiers here
    if (sig.e3 == group.mul(group.exp(mpk.n, gUserID), t))
      return true;
    else
      return false;
  }
  void set_up()
  {
    mpk.l = 4;
    string g = "021780706f0a7afbfc7e5c2fde178c4a0fa86ff9612c233743cadc96c2e85b99eb000000000000000a00000000000000736369656e636500207e580300000000f03b9904000000000384b2c61a7f00006077e8c61a7f0000e05429c61a7f00000a000000000000004b33b2c61a7f0000207e5803000000006077e8c61a7f0000a0";
    string g2 = "02675cbaedec16f50d576a451f813e28d235f8176ab3c748c3e7071197c9c300e404674f3d853347d6f91532a2ca2d27697324275c63c8ed3b8931458e21c394753b9904000000000384b2c61a7f00006077e8c61a7f0000e05429c61a7f00000a000000000000004b33b2c61a7f0000207e5803000000000fab4dc81a7f000043";
    string hibeg1 = "022fcc465af6de5155ecbe906c11a221e6c4ea065c94b64f9c128c69e45127a7f65429c61a7f00000700000000000000f03b9904000000007a5c55c71a7f000067687a020000000000000000000000006077e8c61a7f0000e05429c61a7f00000100000000000000207e580300000000207e580300000000000fbb96801eca69a0";
    string u0 = "038c714d1457e3630c45d809b7a9db159a7d18d4a2e981f492b017a44d75082ede4d4e0d863b3e77731c85a74acb8c187be285a03d4b1cb79b5c8b099ea3ce656bd2c902000000000000000000000000287e580300000000c66155c71a7f00000100000000000000207e580300000000207e5803000000000fab4dc81a7f000038";
    string u1 = "033f62f51761b537d105b9343871fe486b582e567bc0f9ee605de4f53f88639022a4cb9b83d85504632770869911cdb388b2b1082168fec8a244f80c44cf22560bd2c902000000000000000000000000287e580300000000c66155c71a7f00000100000000000000207e580300000000207e5803000000000fab4dc81a7f0000a0";
    string u2 = "0391aaf47f42654a1c1cac4267b0e3df1985da3e0c341d8ab12d29597af64bddcd6657d290c86175522cd2da1c7652e4fbd381445332d6b5977550abf320c0fb98d2c902000000000000000000000000287e580300000000c66155c71a7f00000100000000000000207e580300000000207e5803000000000fab4dc81a7f000060";
    string u3 = "035a3ed74532bda53e84441f4571088468b458c4b83cf2294e4360b85468f6f6835a69507209cfbabbc105f1b8d4fdeecf06093d11c773e3bfa16df653a93fdb10d2c902000000000000000000000000287e580300000000c66155c71a7f00000100000000000000207e580300000000207e5803000000000fab4dc81a7f000065";
    string u4 = "03023afaeeb25eb44de70edee8b47f24f681592f820c4e5874837fb09f2bdfb05c3661488e917ea489240c3f2b2f4c202ab314f4ad0281cd611e90e7568d584f25d2c902000000000000000000000000287e580300000000c66155c71a7f00000100000000000000207e580300000000207e5803000000000fab4dc81a7f0000c3";
    string n = "0066f837d6aaf4d69618917009d0b3c61dc670e614a50d98788cd22400f93c6f22fc9fd14feaff20528338278548c68b71f2a60caed5c8568a61301de0c3256997d26fcf602973721435f651bc6ca3d9230ad04d0b261ae18ca2ab9ae3de01097d518908191408010a85b1ef849579f68286da897c699f394fc48cfb8c1ce3e4a32fa6404a88d40b6d6f571434d7fff3a376c16f25848e8cc3a3cd09236dead65ad8203d97d42b68e76bd2dda61e4edebd1dac6ef620af540bb5a776720633537808a32b1f57b7427849becb1ad34577f089fa78e471fc273d9c6ed9b950aaec23f2be2d40fdb004b6ab3b16c7550eaebc585921acc0acf8eefc928356bdf553801800f40c7f0000207e580300000000287e580300000000b03a990400000000c05429c61a7f0000020000000000000010040000000000007a5c55c71a7f0000d59991020000000000000000000000006077e8c61a7f0000e05429c61a7f00000100000000000000207e580300000000207e580300000000000fbb96801eca69";
    string smsk = "02852d3c40a12f7b1b1d930b0324d90c7a2bd28b4eda25e0210318d2c4d9b33eacb32c094e3b48c78cfaf99272b69c5004b072e725145d1624ed35177810e022d8687a020000000000000000000000006077e8c61a7f0000e05429c61a7f00000100000000000000207e580300000000207e5803000000000fab4dc81a7f000083";
    mpk.g = binToG1(g);
    mpk.g2 = binToG2(g2);
    mpk.hibeg1 = binToG1(hibeg1);
    mpk.hG2.push_back(binToG2(u0));
    mpk.hG2.push_back(binToG2(u1));
    mpk.hG2.push_back(binToG2(u2));
    mpk.hG2.push_back(binToG2(u3));
    mpk.hG2.push_back(binToG2(u4));
    mpk.n = binToGT(n);
    msk = binToG2(smsk);
    //setup(mpk, msk);
  }

  relicxx::G2 getGsk() const
  {
    //此处需读取数据库返回群私钥
    return group.randomG2();
  }
  string g1ToBin(relicxx::G1 g) const
  {
    int len = 4 * FP_BYTES + 1;
    uint8_t bin[len];
    int l;
    l = g1_size_bin(g.g, 1);
    g1_write_bin(bin, l, g.g, 1);
    //bin to str
    string str = "";
    for (int i = 0; i < len; i++)
    {
      int m = atoi(to_string((unsigned int)bin[i]).c_str());
      const char *a = inttohex(m);
      str += a;
    }

    return str;
  }
  string g1ToStr(relicxx::G1 g)
  {
    string str = "";
    auto data = g.getBytes();
    stringstream ss;
    for (auto i : data)
    {
      ss << std::hex << (unsigned int)data[i];
    }
    str = ss.str();

    return str;
  }
  relicxx::G1 binToG1(string str) const
  {
    relicxx::G1 g;
    relicxx::G1 g2 = group.randomG1();
    int len = 4 * FP_BYTES + 1;
    int l;
    l = g1_size_bin(g2.g, 1);
    uint8_t bin[len];
    for (unsigned int i = 0; i < str.length(); i += 2)
    {
      std::string pair = str.substr(i, 2);
      bin[i / 2] = ::strtol(pair.c_str(), 0, 16);
    }
    g1_read_bin(g.g, bin, l);

    return g;
  }
  string g2ToBin(relicxx::G2 g) const
  {
    int len = 4 * FP_BYTES + 1;
    uint8_t bin[len];
    int l;
    l = g2_size_bin(g.g, 1);
    g2_write_bin(bin, l, g.g, 1);

    //bin to str
    string str = "";
    for (int i = 0; i < len; i++)
    {
      int m = atoi(to_string((unsigned int)bin[i]).c_str());
      const char *a = inttohex(m);
      str += a;
    }

    return str;
  }
  string g2ToStr(relicxx::G2 g)
  {
    string str;
    auto data = g.getBytes();
    stringstream ss;
    for (auto i : data)
    {
      ss << std::hex << (unsigned int)data[i];
    }
    str = ss.str();

    return str;
  }
  relicxx::G2 binToG2(string str) const
  {
    relicxx::G2 g;
    relicxx::G2 g2 = group.randomG2();
    int len = 4 * FP_BYTES + 1;
    int l;
    l = g2_size_bin(g2.g, 1);
    uint8_t bin[len];
    for (unsigned int i = 0; i < str.length(); i += 2)
    {
      std::string pair = str.substr(i, 2);
      bin[i / 2] = ::strtol(pair.c_str(), 0, 16);
    }
    g2_read_bin(g.g, bin, l);

    return g;
  }
  string gtToBin(relicxx::GT g) const
  {
    int len = 12 * PC_BYTES;
    uint8_t bin[len];
    int l;
    l = gt_size_bin(g.g, 1);
    gt_write_bin(bin, l, g.g, 1);

    //bin to str
    string str = "";
    for (int i = 0; i < len; i++)
    {
      int m = atoi(to_string((unsigned int)bin[i]).c_str());
      const char *a = inttohex(m);
      str += a;
    }

    return str;
  }
  string gtToStr(relicxx::GT g)
  {
    string str;
    auto data = g.getBytes();
    stringstream ss;
    for (auto i : data)
    {
      ss << std::hex << (unsigned int)data[i];
    }
    str = ss.str();

    return str;
  }
  relicxx::GT binToGT(string str) const
  {
    relicxx::GT g;
    relicxx::GT g2 = group.randomGT();
    int len = 12 * PC_BYTES;
    int l;
    l = gt_size_bin(g2.g, 1);
    uint8_t bin[len];
    for (unsigned int i = 0; i < str.length(); i += 2)
    {
      std::string pair = str.substr(i, 2);
      bin[i / 2] = ::strtol(pair.c_str(), 0, 16);
    }
    gt_read_bin(g.g, bin, l);

    return g;
  }
  string zrToStr(relicxx::ZR zr) const
  {
    int len = CEIL(RELIC_BN_BITS, 8);
    uint8_t bin[RELIC_BN_BITS / 8 + 1];
    bn_write_bin(bin, len, zr.z);

    //bin to str
    string str = "";
    for (int i = 96; i < len; i++)
    {
      int m = atoi(to_string((unsigned int)bin[i]).c_str());
      const char *a = inttohex(m);
      str += a;
    }

    return str;
  }
  relicxx::ZR strToZR(string str) const
  {
    int len = CEIL(RELIC_BN_BITS, 8);
    relicxx::ZR zr;
    uint8_t bin2[RELIC_BN_BITS / 8 + 1];
    for (int i = 0; i < 96; i++)
      bin2[i] = '\0';
    for (unsigned int i = 0; i < str.length(); i += 2)
    {
      std::string pair = str.substr(i, 2);
      bin2[i / 2 + 96] = ::strtol(pair.c_str(), 0, 16);
    }
    bn_read_bin(zr.z, bin2, len);

    return zr;
  }

  char *inttohex(int a) const
  {
    char *buffer = new char[3];
    if (a / 16 < 10)
      buffer[0] = a / 16 + '0';
    else
      buffer[0] = a / 16 - 10 + 'a';
    if (a % 16 < 10)
      buffer[1] = a % 16 + '0';
    else
      buffer[1] = a % 16 - 10 + 'a';
    buffer[2] = '\0';
    return buffer;
  }
};
} // namespace detail

sig_by_key_api::sig_by_key_api() : my(new detail::sig_by_key_api_impl())
{
  JSON_RPC_REGISTER_API(STEEM_sig_by_key_api_plugin_NAME);
}

sig_by_key_api::~sig_by_key_api() {}

// 需要注意创建sig_by_key的时机，因W为sig_by_key的构造函数中会调用JSON RPC插件去注册API，因此
// 需要等JSON RPC先初始化好，plugin_initialize被调用时，会先注册sig_by_key_api_plugin的依赖
// 模块，因此可以确保此时JSON RPC插件此时已经注册完毕。
void sig_by_key_api_plugin::plugin_initialize(const appbase::variables_map &options)
{
  api = std::make_shared<sig_by_key_api>();
}

DEFINE_LOCKLESS_APIS(sig_by_key_api, (set_group)(join_group)(get_sig)(open_paper)(test)(verify_user))
} // namespace sig_by_key
} // namespace plugins
} // namespace steem
