# 9cc

## 参考資料

* [低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook/)
* [C11言語仕様](http://port70.net/~nsz/c/c11/n1570.html)
* [X86 psABI](https://github.com/hjl-tools/x86-psABI)
* [インテル関連ドキュメント・リンク集](https://gist.github.com/tenpoku1000/2250ec65264ff2d639ddeeffd305fe68)
* [お薦めのコンパイラの本とか](https://keens.github.io/blog/2019/02/16/osusumenokonpairanohontoka/)

## 環境

```
$ docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
$ docker run --rm -it -v $PWD:/home/user/work -w /home/user/work compilerbook
```
