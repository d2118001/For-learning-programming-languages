package main

import (
	"fmt"
	"go/token"
	"go/types"
)

func main() {

	fmt.Println("数式を入力(例:1+1)")
	var str string
	fmt.Scan(&str) //標準入力

	last := str[len(str)-1] //文字列の末尾

	//文字列の末尾が数字、)以外の場合削除
	if last != 41 && (57 < last || last < 48) {
		str2 := []rune(str)
		str = string(str2[0 : len(str2)-1])
	}

	//eval関数
	result, err := types.Eval(token.NewFileSet(), nil, token.NoPos, str)

	//0除算などのエラー発生時
	if err != nil {
		fmt.Printf("%s\n", err)
		return
	}

	fmt.Println(result.Value) //答え出力
}