" Compile and run
inoremap <F5> <Esc>:wa<CR>:!cd ../proj && cmake -G "Ninja" . && ninja Robograde && cd ../bin/linux/Debug/ && ./Robograde<CR>
noremap <F5> :wa<CR>:!cd ../proj && cmake -G "Ninja" . && ninja Robograde && cd ../bin/linux/Debug && ./Robograde<CR>
" Compile and output to /dev/pts/1
inoremap <C-F5> <Esc>:wa<CR>:!cd ../proj && cmake -G "Ninja" . && ninja Robograde && cd ../bin/linux/Debug/ && ./Robograde/dev/pts/1<CR>
noremap <C-F5> :wa<CR>:!cd ../proj && cmake -G "Ninja" . && ninja Robograde && cd ../bin/linux/Debug && ./Robograde > /dev/pts/1<CR>
" Compile
inoremap <F7> <Esc>:wa<CR>:!cd ../proj/ && cmake -G "Ninja" . && ninja Robograde<CR>
noremap <F7> :wa<CR>:!cd ../proj/ && cmake -G "Ninja" . && ninja Robograde<CR>

command MS mks! ~/.vim/sessions/Largegame.vim

let g:formatprg_cpp = "astyle"
let g:formatprg_args_cpp = "--options=.codestyle.astyle"
let g:formatprg_h = "astyle"
let g:formatprg_args_h = "--options=.codestyle.astyle"
let g:formatprg_hpp = "astyle"
let g:formatprg_args_hpp = "--options=.codestyle.astyle"
