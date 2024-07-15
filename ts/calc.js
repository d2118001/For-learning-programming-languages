var calculated = false; //計算済みフラグ
//ディスプレイに表示される文字列
function display() {
    var element = document.getElementById("disp");
    return element;
}
function calulation() {
    var dvalue = display().value;
    //文字列の末尾が記号なら計算しない
    if (isNaN(Number(dvalue.slice(-1))))
        return;
    //evalを実行
    try {
        display().value = eval(dvalue);
    }
    catch (error) {
        console.log(error);
    }
    finally {
        calculated = true;
        //0除算エラーがないので出力される文字列を削除
        if (display().value == "Infinity") {
            console.log("divede by zero");
            clear_display();
        }
    }
}
function clear_display() {
    display().value = ""; //Cが押されるか、計算後に数値を入力したら削除
}
function press_num(btn) {
    //計算済みフラグが立っていたらディスプレイをクリアしてから数値を表示
    if (calculated == true) {
        clear_display();
        calculated = false;
    }
    display().value += btn.value; //文字列の最後尾に追記
}
function press_symbol(btn) {
    var sym = btn.value; //入力された記号
    var dvalue = display().value; //ディスプレイに表示されている文字
    //計算済みフラグが立っていたら計算を続けられるように切り替える
    if (calculated == true)
        calculated = false;
    //1文字も入力していなければ何もしない
    if (dvalue == "")
        return;
    //文字列の末尾が記号の場合、2重にならないようにする
    if (isNaN(Number(dvalue.slice(-1)))) {
        dvalue = dvalue.slice(0, dvalue.length - 1);
        dvalue += sym;
        display().value = dvalue;
        return;
    }
    //+と-はそのまま、×と÷は計算できるように文字を変更
    if (sym == "×") {
        sym = "*";
    }
    else if (sym == "÷") {
        sym = "/";
    }
    display().value += sym;
}
