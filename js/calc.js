calculated = false //計算済みフラグ

function calulation(){
    dvalue = document.calculator.display.value //ディスプレイに表示されてる文字

    //文字列の末尾が記号なら計算しない
    if(isNaN(dvalue.substr(-1)))
        return

    //evalを実行
    try {
        document.calculator.display.value = eval(dvalue);
    } catch (error) {
        console.log(error);
    } finally{
        calculated = true
        //0除算エラーがないので出力される文字列を削除
        if(document.calculator.display.value == "Infinity"){
            console.log("divede by zero");
            clear_display()
        }
    }
}

function clear_display(){
    document.calculator.display.value = ""; //Cが押されるか、計算後に数値を入力したら削除
}

function press_num(btn){
    //計算済みフラグが立っていたらディスプレイをクリアしてから数値を表示
    if (calculated == true){
        clear_display()
        calculated =false
    }
    document.calculator.display.value += btn.value; //文字列の最後尾に追記
}

function press_symbol(btn){
    sym = btn.value //入力された記号
    dvalue = document.calculator.display.value //ディスプレイに表示されてる文字

    //計算済みフラグが立っていたら計算を続けられるように切り替える
    if(calculated == true)
        calculated =false

    //1文字も入力していなければ何もしない
    if(dvalue == "")
        return

    //文字列の末尾が記号の場合、2重にならないようにする
    if(isNaN(dvalue.substr(-1))){
        dvalue = dvalue.slice(0,dvalue.length -1);
        dvalue += sym
        document.calculator.display.value = dvalue
        return
    }

    //+と-はそのまま、×と÷は計算できるように文字を変更
    if (sym == "×"){
        sym = "*";
    }else if (sym == "÷"){
        sym = "/";
    }
    document.calculator.display.value += sym;
}