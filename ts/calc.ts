let calculated = false //計算済みフラグ

//ディスプレイformの要素
function display(){
    const element: HTMLInputElement =<HTMLInputElement>document.getElementById("disp");
    return element
}

function calulation(){
    const disp: HTMLInputElement = display()
    //文字列の末尾が記号なら計算しない
    if(isNaN(Number(disp.value.slice(-1))))
        return

    //evalを実行
    try {
        disp.value = eval(disp.value);
    } catch (error) {
        console.log(error)
    } finally{
        calculated = true   //計算失敗してもしなくても計算済みにする
        //0除算エラーがないので出力される文字列を変更
        if(disp.value == "Infinity"){
            disp.value = "0で割ることはできません"
            console.log("diveded by zero.");
        }
    }
}

function clear_display(){
    display().value = ""; //Cが押されるか、計算後に数値を入力したら削除
}

function press_num(btn: HTMLInputElement){
    //計算済みフラグが立っていたらディスプレイをクリアしてから数値を表示
    if (calculated == true){
        clear_display()
        calculated =false
    }
    display().value += btn.value; //文字列の最後尾に追記
}

function press_symbol(btn: HTMLInputElement){
    const disp: HTMLInputElement = display()
    let sym = btn.value //入力された記号
    
    //1文字も入力していなければ何もしない
    if(disp.value == "")
        return

    //計算済みフラグが立っていたら計算を続けられるように切り替える
    if(calculated == true){
        calculated =false
        //計算結果に文字列が入っていたら(計算エラー時)ディスプレイをクリア
        if(isNaN(Number(disp.value.slice(-1)))){
            clear_display()
            return
        }
    }

    //+と-はそのまま、×と÷は計算できるように文字を変更
    if (sym == "×"){
        sym = "*";
    }else if (sym == "÷"){
        sym = "/";
    }

    //文字列の末尾が記号の場合、2重にならないようにする
    if(isNaN(Number(disp.value.slice(-1)))){
        disp.value = disp.value.slice(0,disp.value.length -1) + sym //文字列末尾を入力された記号に変える
        return
    }
    
    disp.value += sym;
}