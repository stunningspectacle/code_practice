syntax on
filetype on
set background=dark
filetype plugin indent on
set number
set title
set ruler
set encoding=utf-8
set fileencoding=utf-8
set nobackup
set hlsearch
set autoindent
set smartindent

set bsdir=buffer
"set autochdir
"set cindent
"set nocompatible

"set tabstop=8 shiftwidth=8

set incsearch
set noexpandtab
set tabstop=8 shiftwidth=8

set encoding=utf-8
set fileencodings=ucs-bom,utf-8,cp936
set fileencoding=gb2312
set termencoding=utf-8

"set tags=tags,./tags,../tags,../../tags,../../../tags,../../../../tags,../../../../../tags,../../../../../../tags
"set tags=./tags,./../tags,./**/tags
"
ab ic set ignorecase
ab tag cs find s
ab nic set noignorecase
ab snn set nonumber
ab sn set number
ab vres vertical resize
ab xxd %!xxd
ab noxxd %!xxd -r
ab setshift8 set tabstop=8 shiftwidth=8
""ab setus set tabstop=4 shiftwidth=4 expandtab


"map <F2> :echo 'Current time is ' . strftime('%c')<CR>
map <F2> :set expandtab tabstop=4 shiftwidth=4<CR>
"
source ~/.vim/cscope_maps.vim

"call plug#begin('~/.vim/plugged')
"
"" Make sure you use single quotes
"
"" Shorthand notation; fetches https://github.com/junegunn/vim-easy-align
"Plug 'junegunn/vim-easy-align'
"
"" Any valid git URL is allowed
"Plug 'https://github.com/junegunn/vim-github-dashboard.git'
"
"" Multiple Plug commands can be written in a single line using | separators
""Plug 'SirVer/ultisnips' | Plug 'honza/vim-snippets'
"
"" On-demand loading
"Plug 'scrooloose/nerdtree', { 'on':  'NERDTreeToggle' }
"Plug 'tpope/vim-fireplace', { 'for': 'clojure' }
"
"" Using a non-master branch
"Plug 'rdnetto/YCM-Generator', { 'branch': 'stable' }
"
"" Using a tagged release; wildcard allowed (requires git 1.9.2 or above)
"Plug 'fatih/vim-go', { 'tag': '*' }
"
"" Plugin options
"Plug 'nsf/gocode', { 'tag': 'v.20150303', 'rtp': 'vim' }
"
"" Plugin outside ~/.vim/plugged with post-update hook
"Plug 'junegunn/fzf', { 'dir': '~/.fzf', 'do': './install --all' }
"
"" Unmanaged plugin (manually installed and updated)
"Plug '~/my-prototype-plugin'
"
"" Initialize plugin system
"call plug#end()
